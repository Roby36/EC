
from typing import List
from enum import Enum
import copy
import time
import logging
import json
import pandas as pd
from typing import Any, Callable, Tuple

from config.validFields import *
from . import cpwalib
from config import globals
from .barFuncs import Bars

class LiveTrade:

    # Required for live trades serialization
    """ IMPORTANT: Since Timestamps are non-json serializable, dates dropped when serializing """
    class dictKeys(Enum):
        BAR         = "bar"
        REASON      = "reason"
        REQ_ORDER   = "requested order"
        EXEC_ORDER  = "executed order"
        CLOSE_DATA  = "close data"
        OPEN_DATA   = "open data"
    
    # Data specific to Live trades
    class LiveTrData:
        def __init__(self, bar: pd.Series, reason: str, req_order: dict, exec_order: pd.DataFrame):
            self.bar, self.reason, self.req_order, self.exec_order = bar, reason, req_order, exec_order

        def to_dict(self) -> dict:
            return {
                # Drop dates from bars (can be recovered later if required given time column)
                LiveTrade.dictKeys.BAR.value:          self.bar.drop(Bars.fields.DATE.value, errors='ignore').to_dict(),
                LiveTrade.dictKeys.REASON.value:       self.reason,
                LiveTrade.dictKeys.REQ_ORDER.value:    self.req_order,
                LiveTrade.dictKeys.EXEC_ORDER.value:   self.exec_order.to_dict()
            }
        
        @classmethod
        def from_dict(cls, data: dict):
            return cls(
                bar         = pd.Series(data.get(LiveTrade.dictKeys.BAR.value,{})),
                reason      = data.get(LiveTrade.dictKeys.REASON.value, ""),
                req_order   = data.get(LiveTrade.dictKeys.REQ_ORDER.value, {}),
                exec_order  = pd.DataFrame(data.get(LiveTrade.dictKeys.EXEC_ORDER.value, {}))
            )

    def __init__(self, bar: pd.Series, reason: str, req_order: dict, exec_order: pd.DataFrame):
        self.open_data = self.LiveTrData(bar, reason, req_order, exec_order)
        self.close_data = None
    
    def isopen(self) -> bool:
        return self.close_data is None 
    
    def close(self, bar: pd.Series, reason: str, req_order: dict, exec_order: pd.DataFrame):
        self.close_data = self.LiveTrData(bar, reason, req_order, exec_order)
    
    def to_dict(self) -> dict:
        return {
            self.dictKeys.OPEN_DATA.value:    self.open_data.to_dict(),
            self.dictKeys.CLOSE_DATA.value:   self.close_data.to_dict() if self.close_data else None
        }

    @classmethod
    def from_dict(cls, data: dict):
        # Start by constructing instance with open data which must be prsent
        open_data = cls.LiveTrData.from_dict(data.get(cls.dictKeys.OPEN_DATA.value, {}))
        instance  = cls(open_data.bar, open_data.reason, open_data.req_order, open_data.exec_order)

        # Next attempt to add close data and add None if this is not available
        close_data_entry = data.get(cls.dictKeys.CLOSE_DATA.value, None)
        instance.close_data = cls.LiveTrData.from_dict(close_data_entry) if close_data_entry is not None else None
        return instance


""" PM class, handling orders """

class Ltpm:

    def __init__(self, 
        acctId: str,
        json_filedir: str,
        # Order retry parameters (assumed to be constant throughout PM operations)
        time_per_attempt: float, max_retries: int, verif_time: float, spm: List[str],
        bt: bool # Set to True if we are in backtesting mode 
    ):
        # Main attribute
        self.trade_data: List[LiveTrade] = []

        self.acctId, self.json_filedir = acctId, json_filedir
        self.time_per_attempt, self.max_retries, self.verif_time, self.spm = time_per_attempt, max_retries, verif_time, spm
        self.bt = bt

    def openTrade(self,
        bar: pd.Series, 
        reason: str, 
        req_order: dict,
        block_duplicates: bool,
        save_updates: bool,
    ) -> int:
        """ 
        Parameters:
            bar: Series holding reference of when trade was first intended to be opened
            reason: reason for attempting to open trade
            req_order: order requested to be executed 
            block_duplicates: True to block if any trade already opened for the same conid at same bar time
            save_updates: True to write to file any updates to trade data

        Further parametrizations / work:
            Mode of order execution (currently IOC implemented)
        
        Return:
            Index of trade in list of trades if successful execution, o/w -1 
        
        """

        if block_duplicates:
            for trade in self.trade_data:
                # Check that both bars contain time information and whether it corresponds
                time_in_indices = Bars.fields.TIME.value in trade.open_data.bar.index and Bars.fields.TIME.value in bar.index
                same_time: bool = trade.open_data.bar[Bars.fields.TIME.value] == bar[Bars.fields.TIME.value] if time_in_indices else False

                # Block order if already filled some opening other order with (1) same conid (2) same bar time
                if (trade.open_data.req_order.get(OrderFields.CONID.value, "") == req_order.get(OrderFields.CONID.value, " ") and same_time):
                    logging.warning(f"LTPM.openTrade() blocked order \n{req_order} \nwith reason {reason} at bar \n{bar}\n"
                                    f" because another trade was opened at same date {pd.to_datetime(bar[Bars.fields.TIME.value], unit=globals.UNIX_TIME_UNITS)}" 
                                    f" with filled order \n{trade.open_data.req_order} \nwith reason {trade.open_data.reason} at bar \n{trade.open_data.bar}\n"
                    )
                    return -1

        if not self.bt:
            cpwalib.suppressMessages(self.spm)
            res_val, exec_order = IOCretryTillFilled(
                self.acctId,
                req_order,
                self.time_per_attempt,
                self.max_retries,
                self.verif_time,
            )
            if not res_val:
                logging.warning(f"LTPM.openTrade() failed to fill order {req_order} with reason {reason} at bar {bar}")
                return -1 
        
        else:
            exec_order = pd.DataFrame()
        
        # Save successfully filled order to list, and return its (immutable) index in the list
        self.trade_data.append(
            LiveTrade(
                bar, reason, req_order, exec_order
            )
        )
        if save_updates:
            self.save_to_json()
        return len(self.trade_data) - 1
    

    def closeTrade(self,
        trade_index: int,
        bar: pd.Series,
        reason: str,
        save_updates: bool,
    ) -> bool:
        """ 
        Parameters:
            trade_index: index of trade to close within trade_data, as returned when calling openTrade()
            bar: Series holding reference of when trade was first intended to be closed 
            reason: reason for attempting to close trade
            save_updates: True to write to file any updates to trade data

        Further parametrizations:
            Mode of order execution (currently IOC implemented)
        
        Return:
            True if trade successfully closed, False if any type of error

        """
        # Ensure validity of requested trade
        if trade_index < 0 or trade_index >= len(self.trade_data):
            logging.warning(f"LTPM.closeTrade(): trade index {trade_index} was given, but currently {len(self.trade_data)} trades are recorded")
            return False        

        # Check that the order was not already closed
        trade: LiveTrade = self.trade_data[trade_index]
        if not trade.isopen():
            logging.warning(f"LTPM.closeTrade(): attempting to close non-open trade {trade_index}")
            return False

        # Retrieve request order, and invert side, keeping all else equal
        req_order: dict = copy.deepcopy(trade.open_data.req_order)
        req_order[OrderFields.SIDE.value] = OrderSide.BUY.value if req_order[OrderFields.SIDE.value] == OrderSide.SELL.value else OrderSide.SELL.value

        if not self.bt:
            # Execute order with same procedure as in openTrade
            cpwalib.suppressMessages(self.spm)
            res_val, exec_order = IOCretryTillFilled(
                self.acctId,
                req_order,
                self.time_per_attempt,
                self.max_retries,
                self.verif_time,
            )
            if not res_val:
                logging.warning(f"LTPM.closeTrade() failed to fill order {req_order} with reason {reason} at bar {bar}")
                return False 
        
        else:
            exec_order = pd.DataFrame()
        
        # Save closing trade data 
        trade.close(bar, reason, req_order, exec_order) 
        if save_updates:
            self.save_to_json()    
        return True
    

    """ Exclusive live trading functionality """
    def save_to_json(self):
        # Convert each LiveTrade instance to a dictionary using to_dict
        data_to_save = [trade.to_dict() for trade in self.trade_data]
        
        # Save the list of dictionaries to a JSON file
        with open(self.json_filedir, 'w') as file:
            json.dump(data_to_save, file, indent=4)  # Using indent for pretty printing


    def load_json(self):
        """ 
        Very ugly function to turn all sort of Exceptions into logging messages 
        """
        try: # Attempt to open and read the JSON file
            with open(self.json_filedir, 'r') as file:
                data_loaded = json.load(file)
        except FileNotFoundError:
            logging.warning(f"Ltpm.load_json() encountered FileNotFoundError when attempting to read trade data from {self.json_filedir}")
            return
        except json.JSONDecodeError:
            logging.warning(f"Ltpm.load_json() encountered JSONDecodeError when attempting to read trade data from {self.json_filedir}")
            return

        # Ignore exceptions from here
        self.trade_data = [LiveTrade.from_dict(trade_dict) for trade_dict in data_loaded if trade_dict is not None]


""" Static functions """

def IOCretryTillFilled(
    acctId: str,
    order: dict,
    time_per_attempt: float,
    retries: int,
    verif_time: float,
    app_coid: str = "-attempt",
) -> Tuple[bool, pd.DataFrame]:
    """
    Resends IOC order until this is completely filled.
    Maximum expected time of execution will be given by retries * (verif_time + time_per_attempt)

    Params:
        order: desired order to be filled
        time_per_attempt: time span between IOC order cancellation reception and next attempt
        retries: number of total retries
        verif_time: time span between order submission and live orders query over which we expect to see order filled by IOC
        app_coid: appended piece to the given order cOID, leading to f"{cOID_base}{app_coid}{retry_num}" effective cOID 

    Return:
        (boolean confirming fully filled order, dataframe with details regarding order executions)

    Assumptions:
        (1) IOC order submitted; otherwise could result in several undesired executions
        (2) order contains valid cOID, else will not be filled due to potential name clashes
        (3) bin/run.sh root/conf.yaml keeps port 5000 open, else will crash
        (4) Message requests must all be suppressed before calling this, else will require further complexity to automate responses
        (5) Response object is valid .json upon successful request
    """

    first_row_val: Callable[[pd.DataFrame, str, Any], Any] = lambda df, col, default: df[col].iloc[0] if (col in df.columns and len(df.index)) else default
    cOID_base: str = order.get(OrderFields.COID.value, "")
    order_responses = pd.DataFrame() # holds all the orders executed

    # Ensure that we have an IOC order
    order[OrderFields.TIME_IN_FORCE.value] = Tif.IOC.value

    # Enter into main retry loop
    for retry_num in range(retries):
        # Modify given order cOID to avoid name clashes
        curr_cOID: str = f"{cOID_base}{app_coid}{retry_num}"
        order[OrderFields.COID.value] = curr_cOID

        # Attempt to place single order
        cpwalib.placeOrder(acctId, [order])
        time.sleep(verif_time)
        req_lo = cpwalib.liveOrders(filters=[], force=False)
        
        # Gracefully extract requested orders dataframe 
        resp: bool = cpwalib.checkResponse("", req_lo)
        lo_dict: dict = req_lo.json() if resp else {}
        orders: List[dict] = lo_dict.get(globals.LIVE_ORDERS_ENTRY, [])
        lo_df = pd.DataFrame(orders) # could be empty if error encountered

        # Sort and orders and filter by cOID
        lo_df = lo_df.sort_values(by=OrderFields.LAST_EXECUTION_TIME.value, ascending=False) if OrderFields.LAST_EXECUTION_TIME.value in lo_df.columns else lo_df
        filt_df = lo_df[lo_df[OrderFields.ORDER_REF.value] == curr_cOID] if OrderFields.ORDER_REF.value in lo_df.columns else pd.DataFrame()

        # Extract relevant attributes of most recent matching order
        filled_quant: float = first_row_val(filt_df, OrderFields.FILLED_QUANTITY.value, 0.0)
        remain_quant: float = first_row_val(filt_df, OrderFields.REMAINING_QUANTITY.value, 0.0)
        status: str         = first_row_val(filt_df, OrderFields.STATUS.value, "status not found")
        orderId: str        = first_row_val(filt_df, OrderFields.ORDERID.value, "")

        # Extract order response and add to order responses
        order_responses = pd.concat([order_responses, filt_df.iloc[[0]]]) if len(filt_df.index) else order_responses

        # Return if necessary fill conditions are met
        if (status.lower() == OrderStatus.FILLED.value.lower() and remain_quant == 0.0 and filled_quant != 0.0):
            return (True, order_responses)
        
        # Cancel the order for redundancy if not filled immediately
        cpwalib.cancelOrder(acctId, orderId)

        # Update remaining quantity for next retry
        order[OrderFields.QUANTITY.value] = int(remain_quant)

        # Sleep until next retry
        time.sleep(time_per_attempt)
    
    return (False, order_responses)


def bracketOrder(
    parent_order: dict, 
    lower_bracket: float,
    higher_bracket: float
) -> List[dict]:
    """ 
    Generates list of three orders defining bracket order for given parent order
    """
    buy: bool = parent_order.get(OrderFields.SIDE.value,"").lower() == OrderSide.BUY.value.lower()
    side: str = OrderSide.SELL.value if buy else OrderSide.BUY.value

    pt, sl = copy.deepcopy(parent_order), copy.deepcopy(parent_order)
    pt.pop(OrderFields.COID.value, None), sl.pop(OrderFields.COID.value, None)
    sl[OrderFields.PARENT_ID.value], pt[OrderFields.PARENT_ID.value] = (parent_order.get(OrderFields.COID.value,""),) * 2
    sl[OrderFields.SIDE.value], pt[OrderFields.SIDE.value] = (side,)*2
    sl[OrderFields.PRICE.value], pt[OrderFields.PRICE.value] = (lower_bracket, higher_bracket) if buy else (higher_bracket, lower_bracket)
    sl[OrderFields.ORDER_TYPE.value], pt[OrderFields.ORDER_TYPE.value] = OrderTypes.STOP.value, OrderTypes.LIMIT.value

    return [parent_order, sl, pt]

