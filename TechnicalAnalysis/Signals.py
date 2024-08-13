
from enum import Enum
import json
import logging
import pandas as pd
from typing import List, Type, Callable, Tuple
from WebAPINotes.barFuncs import Bars
from config.validFields import OrderSide

from . import techInds as Indicators

class Sigs:

    # Define signal types
    class Open(Enum):
        BUY = "BUY"
        SELL_SHORT = "SELL SHORT"
        HOLD = "HOLD"
    
    class Close(Enum):
        BUY_TO_COVER = "BUY TO COVER"
        SELL = "SELL"
        HOLD = "HOLD"
    
    class Funcs(Enum):
        NEG_DIV = "neg divergence"
        NEG_DIV_NEG_MAX_AND_RBP_IN_LADDER = "neg div AND neg max AND right max in ladder"
        NEG_DIV_NEG_MAX_OR_RBP_IN_LADDER = "neg div AND neg max OR right max in ladder"

        OPP_DIV = "opposite divergence"
        BOLL_CROSSING = "bollinger bands crossing"
        NEG_TRADE_EXP = "negative trade expiration"
        BRACKETS = "Take profit or Stop loss"
    
    openSig  = Callable[[pd.Series], Tuple[Open, int]] 
    closeSig = Callable[[pd.Series, pd.Series, OrderSide], Close] 

    # Instance attributes
    def __init__(self, 
                
        # Fixed indicator settings
        RSI_per: int,
        boll_per:  int,
        boll_mult: float,
        max_div_per: int,
        max_neg_per: int,
        ladd_min_stat_pts: int,
        ladd_max_stat_pts: int,

        # Trading settings
        stop_loss: float,
        take_profit: float,
        exp_bars: int,
    ):
        # Central bar information
        self.bar_agg: Indicators.BarAgg = None
        self.bars: pd.DataFrame = None

        # Construct fixed dictionary of **settings to be passed when computing each indicator
        self.ind_settings:  dict[Type[Indicators.TechInd], dict] = {
            Indicators.RSI: {
                "period": RSI_per,
            },
            Indicators.BollBands: {
                "period": boll_per,
                "mult": boll_mult,
            },
            Indicators.Div: {
                "RSI_period": RSI_per,
                "max_div_period": max_div_per,
                "max_neg_period": max_neg_per,
            },
            Indicators.Ladders: {
                "min_stat_pts": ladd_min_stat_pts,
                "max_stat_pts": ladd_max_stat_pts,
            }
        }
        # Hold corresponding dataframes returned by Indicators.TechInd.compute() when called with ind_settings
        self.ind_data : dict[Type[Indicators.TechInd], pd.DataFrame] = {}

        # Define accessor for signal functions
        self.open_sig_funcs: dict[Sigs.Funcs, Sigs.openSig] = {
            Sigs.Funcs.NEG_DIV:           self.neg_div,
            Sigs.Funcs.NEG_DIV_NEG_MAX_AND_RBP_IN_LADDER: self.neg_div_neg_max_and_rbp_in_ladder,
            Sigs.Funcs.NEG_DIV_NEG_MAX_OR_RBP_IN_LADDER:  self.neg_div_neg_max_or_rbp_in_ladder,
        }
        self.close_sig_funcs: dict[Sigs.Funcs, Sigs.closeSig] = {
            Sigs.Funcs.OPP_DIV:         self.opposite_div,
            Sigs.Funcs.BRACKETS:        self.enforce_brackets,
            Sigs.Funcs.BOLL_CROSSING:   self.boll_crossing,
            Sigs.Funcs.NEG_TRADE_EXP:   self.neg_trade_exp,
        }

        self.stop_loss, self.take_profit, self.exp_bars = stop_loss, take_profit, exp_bars
    
    def compute_indicators(self,
        bars: pd.DataFrame,
        barAgg: Indicators.BarAgg,
    ):
        """ 
        Updates indicators for new bar data provided by the caller
        """
        # Keep track of most recent bar data
        self.bar_agg = barAgg
        self.bars  = bars
        # Recompute from scratch each indicator using provided bar data
        for indicator_class, settings in self.ind_settings.items():
            self.ind_data[indicator_class] = indicator_class.compute(bars, barAgg, **settings)
    

    """ Helpers """
    def loc_bar(self, target_bar: pd.Series) -> int:
        """ 
        Locates a bar in bars dataframe and performs preliminary checks, and returns its corresponding index in the current dataframe
        """
        # Check that bar and indicator data properly provided
        if self.bar_agg is None or self.bars is None or not self.ind_data:
            logging.warning("loc_bar_checks(): No bar or indicator data. Please provide bar data by calling compute_indicators")
            return -1 
        # Use unix time as exclusive identifier to locate bar within new dataframe
        res_df: pd.DataFrame = self.bars[self.bars[Bars.fields.TIME.value] == target_bar[Bars.fields.TIME.value]]
        if res_df.empty:
            logging.warning("loc_bar_checks(): No bar matching target bar found in given dataframe")
            return -1
        return res_df.index[0]
    
    def balance(self, open_index: int, curr_index: int, open_order_side: OrderSide) -> float:
        if open_index < 0 or curr_index < 0 or open_index >= len(self.bars) or curr_index >= len(self.bars):
            logging.warning(f"Signals.balance() received out-of-range open_index = {open_index} or curr_index = {curr_index}")
            return 0.0
        open_bar, curr_bar = self.bars.iloc[open_index], self.bars.iloc[curr_index]
        return (self.bar_agg(curr_bar) - self.bar_agg(open_bar)) * (1 if open_order_side == OrderSide.BUY else -1) 
    

    """ Open signals functions definitions """
    def opensignal(func):
        def wrapper(self, curr_bar: pd.Series) -> 'Tuple[Sigs.Open, int]':
            curr_index : int = self.loc_bar(curr_bar)
            if (curr_index == -1): return (Sigs.Open.HOLD, 0)
            return func(self, curr_index)
        wrapper.internal = func
        return wrapper
    
    @opensignal
    def neg_div(self, curr_index: int) -> Tuple[Open, int]:
        """ NOTE: Signal issued at bar i iff neg divergence recorded at bar (i - 1) """
        div_ind: pd.DataFrame = self.ind_data[Indicators.Div]
        sel_div: pd.DataFrame = div_ind[div_ind[Indicators.Div.fields.NEG.value] == (curr_index - 1)]
        if sel_div.empty: return (Sigs.Open.HOLD, 0)
        div_nat: int = sel_div[Indicators.Div.fields.M.value].sum()
        return ((Sigs.Open.BUY if div_nat > 0 else Sigs.Open.SELL_SHORT), len(sel_div))
    
    """ NOTE: Undecorated methods: MUST be called INSIDE of decorated methods """
    def in_ladder(self, index: int, req_dir: Indicators.Ladders.dir) -> bool:
        """ Determine if some index is located in some ladder of given direction """
        ladders_ind: pd.DataFrame = self.ind_data[Indicators.Ladders]
        in_range = (index >= ladders_ind[Indicators.Ladders.fields.LEFT_STAT.value]) & \
                   (index <= ladders_ind[Indicators.Ladders.fields.RIGHT_STAT.value])
        direction_match = ladders_ind[Indicators.Ladders.fields.DIRECTION.value] == req_dir.value
        is_contained: bool = (in_range & direction_match).any()
        return is_contained
    
    def neg_div_in_ladder(self, curr_index: int) -> Tuple[Tuple[Open, int], bool, bool, bool]:
        """  NOTE: Checks the following
            (1) neg divergence at bar (i - 1) 
            (2) (i - 1) contained in some ladder
            (3) for some neg div at (i - 1), we can find some R bar contained in some ladder
            (4) for some neg div at (i - 1), we can find some L bar contained in some ladder
        """
        # Helper
        div_field_in_ladder = lambda field, div_df, req_dir: (
            div_df[field.value].apply(lambda f: self.in_ladder(f, req_dir)).any() if not div_df.empty else False
        )

        # (1) Check neg div elementary signal
        sig_dir, sig_weight = self.neg_div.internal(self, curr_index) # decorator bypass     
        if sig_dir == Sigs.Open.HOLD: return ((sig_dir, sig_weight), False, False, False) # No neg div found

        # (2) (3) (4) Check if neg stat and some lbp, rbp in ladder, in required direction
        req_dir = Indicators.Ladders.dir.UP if sig_dir == Sigs.Open.SELL_SHORT else Indicators.Ladders.dir.DOWN        
        neg_divs: pd.DataFrame    = self.ind_data[Indicators.Div][self.ind_data[Indicators.Div][Indicators.Div.fields.NEG.value] == (curr_index - 1)]
        neg_stat_in_ladder: bool  = self.in_ladder(curr_index - 1, req_dir)
        rbp_in_ladder: bool       = div_field_in_ladder(Indicators.Div.fields.RBP, neg_divs, req_dir)
        lbp_in_ladder: bool       = div_field_in_ladder(Indicators.Div.fields.LBP, neg_divs, req_dir)

        return ((sig_dir, sig_weight), lbp_in_ladder, rbp_in_ladder, neg_stat_in_ladder)


    @opensignal
    def neg_div_neg_max_and_rbp_in_ladder(self, curr_index: int) ->  Tuple[Open, int]:
        ((sig_dir, sig_weight), lbp_in_ladder, rbp_in_ladder, neg_stat_in_ladder) = self.neg_div_in_ladder(curr_index)
        return (sig_dir, sig_weight) if rbp_in_ladder and neg_stat_in_ladder else (Sigs.Open.HOLD, 0)
    
    @opensignal
    def neg_div_neg_max_or_rbp_in_ladder(self, curr_index: int) ->  Tuple[Open, int]:
        ((sig_dir, sig_weight), lbp_in_ladder, rbp_in_ladder, neg_stat_in_ladder) = self.neg_div_in_ladder(curr_index)
        return (sig_dir, sig_weight) if rbp_in_ladder or neg_stat_in_ladder else (Sigs.Open.HOLD, 0)

    """ Close signals functions definitions """
    closing_sig: dict[OrderSide, Close] = {
        OrderSide.SELL: Close.BUY_TO_COVER,
        OrderSide.BUY:  Close.SELL,
    }

    def closesignal(func):
        def wrapper(self, open_bar: pd.Series, curr_bar: pd.Series, open_order_side: OrderSide) -> 'Sigs.Close':
            open_index, curr_index = self.loc_bar(open_bar), self.loc_bar(curr_bar)
            if open_index == -1 or curr_index == -1: return Sigs.Close.HOLD
            return func(self, open_index, curr_index, open_order_side)
        return wrapper

    @closesignal
    def opposite_div(self, open_index: int, curr_index: int, open_order_side: OrderSide) -> Close:
        """ NOTE: Signal issued at bar i iff opposite divergence recorded at bar (i - 1) """
        div_ind: pd.DataFrame = self.ind_data[Indicators.Div]
        sel_div: pd.DataFrame = div_ind[div_ind[Indicators.Div.fields.RBP.value] == (curr_index - 1)]
        if sel_div.empty: return Sigs.Close.HOLD 
        div_nat: int = sel_div[Indicators.Div.fields.M.value].sum()
        return (
            Sigs.closing_sig[open_order_side] if (
                div_nat > 0 and open_order_side == OrderSide.SELL or 
                div_nat < 0 and open_order_side == OrderSide.BUY) else 
            Sigs.Close.HOLD
        )
    
    @closesignal
    def boll_crossing(self, open_index: int, curr_index: int, open_order_side: OrderSide) -> Close:
        """ NOTE: Signal issued at bar i iff crossing recorded at same bar i """ 
        cross_res: List[bool] = []
        for band, dir in [(Indicators.BollBands.fields.BOLL_UPPER, Indicators.BollBands.crossDir.DOWN),
                          (Indicators.BollBands.fields.BOLL_LOWER, Indicators.BollBands.crossDir.UP)]:
            cross_res.append(
                Indicators.BollBands.cross(
                    bars=self.bars, barAgg=self.bar_agg,
                    BollBands=self.ind_data[Indicators.BollBands],
                    index=curr_index, band=band, dir=dir,
                )
            )
        # Issue signals only if crossings occured and in profit
        cross_down, cross_up = cross_res
        if self.balance(open_index, curr_index, open_order_side) < 0: 
            return Sigs.Close.HOLD
        return (
            Sigs.closing_sig[open_order_side] if ( 
                cross_down and open_order_side == OrderSide.BUY or
                cross_up and open_order_side == OrderSide.SELL) else 
            Sigs.Close.HOLD
        )
    
    @closesignal
    def neg_trade_exp(self, open_index: int, curr_index: int, open_order_side: OrderSide) -> Close:
        if self.balance(open_index, curr_index, open_order_side) > 0 or curr_index - open_index < self.exp_bars:
            return Sigs.Close.HOLD
        return Sigs.closing_sig[open_order_side]
    
    @closesignal
    def enforce_brackets(self, open_index: int, curr_index: int, open_order_side: OrderSide) -> Close:
        perc_change: float = self.balance(open_index, curr_index, open_order_side) / self.bar_agg(self.bars.iloc[open_index])
        close = perc_change > self.take_profit or -perc_change > self.stop_loss
        return Sigs.closing_sig[open_order_side] if close else Sigs.Close.HOLD

        
