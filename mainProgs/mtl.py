
""" 
This file holds the main trading program, connecting the Trade/ API-dependent functionality in WebAPINotes
with the independent Techinal Analysis Indicators Signals
"""
import pandas as pd
from typing import List 
import logging 
from typing import Tuple
import copy
import time
import itertools

from config.validFields import OrderFields, OrderSide
from TechnicalAnalysis.Signals import Sigs
from WebAPINotes.pms import Ltpm, LiveTrade
from TechnicalAnalysis.techInds import BarAgg
from WebAPINotes.barFuncs import Bars
from config import globals

def trade_loop(
    # Backetsting & Live params
    sigs_params: dict,
    pm_params: dict,
    barAgg: BarAgg,
    open_sigs: List[Sigs.Funcs],
    close_sigs: List[Sigs.Funcs],
    order_fields: dict,
    base_order_quant: int,
    flat_order_quant: bool,
    write_trading_updates: bool,
    bt: bool,

    # Live trading only params
    reqBars_args: dict | None = None,
    bar_size: pd.Timedelta | None = None,
    updates_per_bar: int | None = None, # How many times per bar period elapsed we attempt to update the bars again

    # Backtesting only params
    bt_bars: pd.DataFrame | None = None, 
    chunk_size: int | None = None,

) -> List[LiveTrade] | None:
    """ 
    Undergoes main trading procedure, parallelizing backtesting and live trading cases
    Returns list of stored trade information (which can then be remapped & saved)
    """

    # Argument check 
    if (bt and (bt_bars is None or chunk_size is None) or not bt and (reqBars_args is None or bar_size is None or updates_per_bar is None)):
        logging.warning("trade_loop() called without the necessary parameters.")
        return None

    # Pass constructor values 
    sigs = Sigs(**sigs_params)
    pm = Ltpm(**pm_params)

    # Main loop
    for curr_it in itertools.count(start = chunk_size if bt else 0, step=1):
        
        if not bt:
            # Retreieve bar data from IBKR
            mdh: pd.DataFrame = Bars.reqBars(**reqBars_args)
            if mdh.empty or Bars.fields.TIME.value not in mdh.index: 
                logging.warning(f"trade_loop(): Bars.reqBars() returned invalid bar data at iteration {curr_it}")
                continue

            # Aggregate bars to desired bar size
            input_bar_size : pd.Timedelta = mdh[Bars.fields.TIME.value].apply(lambda d: pd.Timestamp(d, unit=globals.UNIX_TIME_UNITS)).diff().min()
            agg_fact: int = int(bar_size.total_seconds() / input_bar_size.total_seconds())
            final_bars: pd.DataFrame = Bars.aggBars(mdh=mdh, agg_fact=agg_fact, bar_size=input_bar_size)
            if final_bars.empty:
                logging.warning(f"trade_loop(): Bars.aggBars() returned invalid bar data at iteration {curr_it}")
                continue
            
            # Protection from duplicate trade openings: at least one full bar period must have elapsed
            last_bar: pd.Series =  final_bars.iloc[-1]
            if pm.trade_data:
                last_trade_bar_time = pd.Timestamp(pm.trade_data[-1].open_data.bar[Bars.fields.TIME.value], units=globals.UNIX_TIME_UNITS)
                last_bar_time = pd.Timestamp(last_bar[Bars.fields.TIME.value], units=globals.UNIX_TIME_UNITS)
                if (last_bar_time - last_trade_bar_time < bar_size):
                    logging.warning(f"trade_loop(): Detected trade opened before one full bar period elapsed at {curr_it}")
                    continue

        else: # if bt
            if curr_it >= len(bt_bars): break
            final_bars: pd.DataFrame = bt_bars[curr_it - chunk_size : curr_it + 1].reset_index(drop=True)
            last_bar: pd.Series = final_bars.iloc[-1]

        # Reconnect bt and live control flows
        sigs.compute_indicators(final_bars, barAgg)

        # Check open signals
        open_sig_results: List[Tuple[Sigs.Funcs, Tuple[Sigs.Open, int]]] = [(sig_func, sigs.open_sig_funcs[sig_func](last_bar)) for sig_func in open_sigs]
        succ_sig_results: List[Tuple[Sigs.Funcs, Tuple[Sigs.Open, int]]] = [(sig_func, (open_sig, weight)) for (sig_func, (open_sig, weight)) in open_sig_results if weight]
        if succ_sig_results: 
            # If open signals issued, assume they must all go in same direction if presen
            cum_sig_weight: int = sum(weight for (_, (_, weight)) in succ_sig_results)
            reason: str         = "; ".join([sig_func.value for (sig_func, (_, _)) in succ_sig_results])
            res_sig: Sigs.Open  = Sigs.Open.BUY if any(open_sig == Sigs.Open.BUY for (_, (open_sig, _)) in succ_sig_results) else Sigs.Open.SELL_SHORT
            open_order: dict = copy.deepcopy(order_fields)
            open_order[OrderFields.SIDE.value]      = OrderSide.BUY.value if res_sig == Sigs.Open.BUY else OrderSide.SELL.value
            open_order[OrderFields.COID.value]      = f"{open_order[OrderFields.COID.value]}-{curr_it}" # generate unique COID
            open_order[OrderFields.QUANTITY.value]  = int(base_order_quant * (1 if flat_order_quant else cum_sig_weight))
            pm.openTrade(last_bar, reason, open_order, block_duplicates=True, save_updates=write_trading_updates)
        
        # Check close signals
        for trade_num, trade in enumerate(pm.trade_data):
            if not trade.isopen(): continue
            open_bar: pd.Series = trade.open_data.bar
            open_order_side = OrderSide.BUY if trade.open_data.req_order.get(OrderFields.SIDE.value, "") == OrderSide.BUY.value else OrderSide.SELL 
            close_sig_results: List[Tuple[Sigs.Funcs, Sigs.Close]] = [(sig_func, sigs.close_sig_funcs[sig_func](open_bar, last_bar, open_order_side)) for sig_func in close_sigs]
            succ_close_sigs:   List[Tuple[Sigs.Funcs, Sigs.Close]] = [(sig_func, close_sig) for (sig_func, close_sig) in close_sig_results if not close_sig == Sigs.Close.HOLD ]
            if not succ_close_sigs: continue 
            reason: str = "; ".join([sig_func.value for (sig_func, _) in succ_close_sigs])
            pm.closeTrade(trade_num, last_bar, reason, save_updates=write_trading_updates)
        
        # End-of-loop procedures 
        logging.info(f"trade_loop(): Completed bar iteration {curr_it}")
        time.sleep(bar_size.total_seconds() / updates_per_bar if not bt else 0)
    
    logging.info(f"trade_loop() terminated ")
    return pm.trade_data



