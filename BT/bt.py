
import pandas as pd 
import logging
from enum import Enum 
import json 
import sys
import os
from typing import Tuple, List

from TechnicalAnalysis.Signals import Sigs
from WebAPINotes.pms import LiveTrade
from mainProgs import mtl
from TechnicalAnalysis.techInds import BarAgg
from WebAPINotes.barFuncs import Bars
from config.validFields import OrderFields, OrderSide, OrderTypes, Tif
from config import globals

bar_close: BarAgg = lambda bar: bar[Bars.fields.CLOSE.value]
bt: bool = True
chunk_size: int = 200

# Get absolute directory to ensure correct file is accessed
current_script_directory = os.path.dirname(os.path.abspath(__file__))
config_file: str = os.path.join(current_script_directory, 'btconfig.json')

class BtParamsKeys(Enum):
    SIGS_PARAMS = "sigs_params"
    OPEN_SIGS   = "open_sigs"
    CLOSE_SIGS  = "close_sigs"

class TradeFields(Enum):
    SIDE = "side"
    ENTRY_DATE = "Entry date"
    ENTRY_REASON = "Entry reason"
    ENTRY_PRICE = "Entry price"
    EXIT_PRICE = "Exit price"
    EXIT_REASON = "Exit reason"
    EXIT_DATE = "Exit date"
    PL = "P&L"
    ENTRY_SIGS = "Entry signals"

pm_params = {
    "acctId":           globals.AccountID.FABIO.value,
    "json_filedir":     "BT Trade Data.json",
    "time_per_attempt": 10.0, 
    "verif_time":       10.0, 
    "max_retries":      8, 
    "spm":              [],# spm.MessageId.tolist(),
    "bt":               bt
}

order_fields : dict = {
    OrderFields.ACCT_ID.value:  globals.AccountID.FABIO.value,
    OrderFields.CONID.value:    346727821, # DAX ETF
    OrderFields.COID.value:     "Dummy bt order", 
    OrderFields.ORDER_TYPE.value: OrderTypes.MARKET.value,
    OrderFields.SIDE.value:     OrderSide.BUY.value,
    OrderFields.TIF.value:      Tif.IOC.value,
    OrderFields.QUANTITY.value: 1,
}

def extract_input_bars(input_filepath : str ) -> pd.DataFrame | None:
    """ Gracefully extract bars from specifically formatted file and process them to the right format """

    # File extraction
    try:
        test_bars: pd.DataFrame = pd.read_excel(input_filepath).dropna(axis=1)
        logging.info(f"Input data {input_filepath} file read successfully.")
    except FileNotFoundError:
        logging.warning(f"Error: Input data {input_filepath}  file does not exist.")
        return None
    except ValueError:
        logging.warning(f"Error: Input data {input_filepath}  fil is of an unsupported file format or is corrupted.")
        return None
    if (Bars.fields.CLOSE.value not in test_bars.columns or Bars.fields.DATE.value not in test_bars.columns):
        logging.warning(f"Critical data columns (date or close values) missing from input {input_filepath}  file")
        return None

    # Bar processing
    test_bars[Bars.fields.TIME.value] = test_bars[Bars.fields.DATE.value].apply(lambda i: int(pd.Timestamp(i).timestamp() * 1000))
    test_bars = test_bars.sort_values(by=Bars.fields.TIME.value).reset_index(drop=True)
    logging.info(f"{input_filepath} processed properly")
    return test_bars 


def load_nested(config_file: str) -> Tuple[dict, List, List] | None:
    """ Function to extract main backtesting settings from input json,  (sigs_params, open_sigs, close_sigs) """
    
    # Handle file opening and parsing
    try: 
        with open(config_file, 'r') as file:
            data_loaded: dict = json.load(file)
    except FileNotFoundError:
        logging.warning(f"Could not find file {config_file}")
        return None
    except json.JSONDecodeError:
        logging.warning(f"Could not properly decode data from file {config_file}")
        return None
    
    # Handle nested structures unpacking
    sigs_params, open_sigs, close_sigs = (
        data_loaded.get(BtParamsKeys.SIGS_PARAMS.value, None),
        data_loaded.get(BtParamsKeys.OPEN_SIGS.value, None),
        data_loaded.get(BtParamsKeys.CLOSE_SIGS.value, None)
    )
    if sigs_params is None or open_sigs is None or close_sigs is None:
        logging.warning("Missing sigs_params, open_sigs, or close_sigs")
        return None
    
    return (sigs_params, open_sigs, close_sigs)


""" Converting signals Enum <-> str """
def sigs_to_str(sigs: List[Sigs.Funcs]):
    return [sig.value for sig in sigs]

def sigs_from_str(sigs: List[str]):
    return [sig for sig in Sigs.Funcs if sig.value in sigs]

def display_trades(trade_data: LiveTrade) -> pd.DataFrame:
    """ Convert LiveTrade object to dataframe representation """
    trades = [
        {
            TradeFields.SIDE.value:         trade.open_data.req_order[OrderFields.SIDE.value],
            TradeFields.ENTRY_DATE.value:   trade.open_data.bar[Bars.fields.DATE.value],
            TradeFields.ENTRY_PRICE.value:  bar_close(trade.open_data.bar),
            TradeFields.ENTRY_REASON.value: trade.open_data.reason,
            TradeFields.ENTRY_SIGS.value:   trade.open_data.req_order[OrderFields.QUANTITY.value],
            TradeFields.EXIT_DATE.value:    trade.close_data.bar[Bars.fields.DATE.value] if trade.close_data else pd.NA,
            TradeFields.EXIT_PRICE.value:   bar_close(trade.close_data.bar) if trade.close_data else pd.NA,
            TradeFields.EXIT_REASON.value:  trade.close_data.reason if trade.close_data else pd.NA,
            TradeFields.PL.value: (
                (bar_close(trade.close_data.bar) - bar_close(trade.open_data.bar)) *
                (1 if trade.open_data.req_order[OrderFields.SIDE.value] == OrderSide.BUY.value else -1)
            ) if trade.close_data else pd.NA
        }
        for trade in trade_data
    ]
    return pd.DataFrame(trades)


def save_output_trades(trades_df: pd.DataFrame, output_excel_filepath: str) -> bool:
    try:
        trades_df.to_excel(output_excel_filepath)
        logging.info(f"Output file {output_excel_filepath} written successfully.")
        return True
    except PermissionError:
        logging.warning(f"Error: Permission denied when trying to write to {output_excel_filepath}.")
        return False
    except FileNotFoundError:
        logging.warning(f"Error: The directory {output_excel_filepath} does not exist.")
        return False


def main():

    # Configure logging to output to stdout
    logging.basicConfig(
        level=logging.INFO,  
        format='%(asctime)s - %(levelname)s - %(message)s',  
        handlers=[
            logging.StreamHandler(sys.stdout) 
        ],
        force=True
    )

    # Check if exactly two arguments are provided (plus the script name)
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input_filepath output_filepath")
        sys.exit(1)  # Exit the script with an error code

    # Extract arguments
    input_filepath = sys.argv[1]
    output_filepath = sys.argv[2]

    # Placeholder for processing the files
    logging.info(f"Input file: {input_filepath}")
    logging.info(f"Output file: {output_filepath}")

    # Bar extraction
    test_bars = extract_input_bars(input_filepath)
    if test_bars is None:
        logging.warning("Error extracting bars")
        exit(2)
    if len(test_bars) < chunk_size:
        logging.warning(f"Input data does not reach the required {chunk_size} minimum datapoints. Please provide larger dataset.")
        exit(3)
    
    # Extract backtesting parameters
    read_res = load_nested(config_file)
    if read_res is None:
        logging.warning(f"Error extracting backtesting parameters from {config_file}")
        exit(4)

    sigs_params, open_sigs, close_sigs = read_res
    open_sigs, close_sigs = sigs_from_str(open_sigs), sigs_from_str(close_sigs)
    if not open_sigs or not close_sigs or not sigs_params:
        logging.warning("Error extracting open_sigs or close_sigs or sigs_params from file: cannot proceed with backtesting")
        exit(5)

    # Convert back to would-be.written-back format (debugging)
    input_params_dict = {
        BtParamsKeys.SIGS_PARAMS.value: sigs_params,
        BtParamsKeys.OPEN_SIGS.value: sigs_to_str(open_sigs),
        BtParamsKeys.CLOSE_SIGS.value: sigs_to_str(close_sigs)
    }
    logging.info(f"Backtesting input parameters: \n\n {input_params_dict} \n\n")

    # Finally proceed to main trading loop
    trade_data: List[LiveTrade] | None = mtl.trade_loop(

        sigs_params=sigs_params,
        pm_params=pm_params,
        barAgg=bar_close,
        open_sigs=open_sigs,
        close_sigs=close_sigs,
        order_fields=order_fields,

        base_order_quant=1,
        flat_order_quant=False,
        write_trading_updates=False,
        bt=bt,
        bt_bars=test_bars,
        chunk_size=chunk_size,
    )

    if trade_data is None:
        logging.warning("Error from trade_loop()")
        exit(6)
    
    trades_df: pd.DataFrame = display_trades(trade_data)
    if trades_df.empty:
        logging.warning("Backtesting produced no trades")
        exit(7)
    
    res: bool = save_output_trades(trades_df, output_filepath)
    if res:
        logging.info("Backtesting procedure terminated correctly")
    

if __name__ == '__main__':
    main()