
import pandas as pd 
from enum import Enum
from typing import Callable, List
from abc import ABC, abstractmethod
from statistics import mean, stdev
from collections import Counter

""" Bar aggregator type alias """
BarAgg = Callable[[pd.Series], float]

""" Abstract Indicator Class"""
class TechInd(ABC):
    
    class fields(Enum):
        pass

    @classmethod
    def index(cls):
        return [field.value for field in cls.fields]
    
    @classmethod
    @abstractmethod
    def compute(cls, bars: pd.DataFrame, barAgg: BarAgg, *args, **kwargs) -> pd.DataFrame:
        pass


""" Elementary Indicators """

class LocStat(TechInd):

    class fields(Enum):
        BI = "bar index"
        LD = "left depth"
        RD = "right depth"
        LC = "left change (%)"
        RC = "right change (%)"

    class typ(Enum):
        MIN = 1
        MAX = -1 
    
    @classmethod
    def compute(cls, bars: pd.DataFrame, barAgg: BarAgg,
        m: typ,
    ) -> pd.DataFrame:
        
        """ 
        Computes the maximum/minimum points for a given bar dataframe

        Parameters:
            bars: source bars dataframe
            m: set to either LocStatType.MAX or LocStatType.MIN depending on whether
                we want to calculate local maxima or minima
            barAgg: desired aggregator function (e.g. bar's close)
        
        Returns:
            Dataframe containing details regarding local maxima or minima in the input dataset
            
        """

        num_bars: int = len(bars)
        d: int = 0
        bar_agg = lambda i: barAgg(bars.loc[i]) # shorthand accessor
        stat_pts: List[pd.Series] = []

        # Lagging indicator: we can only declare statPoints up to one bar before current
        while (d < num_bars - 1):
            ld, rd = 0, 0
            i: int = d

            # Slide to the right until function is uniform
            while (i < (num_bars - 1) and m.value * bar_agg(i) > m.value * bar_agg(i + 1)):
                ld, i = ld + 1, i + 1
            
            # Exit loop if we exhausted bars
            if i >= (num_bars - 1):
                d = i # this ensures we also break outer loop 
                continue 

            # Exit loop if no increment/decrement
            if (ld == 0):
                d += 1
                continue

            # If we made it to here, then we have identified a local stationary point
            stat = pd.Series(index = cls.index())
            stat[cls.fields.BI.value] = i 
            stat[cls.fields.LD.value] = ld 
            stat[cls.fields.LC.value] = -100 * m.value * (bar_agg(i) - bar_agg(i - ld)) / bar_agg(i)

            # Slide to the right expecting opposite direction
            while (i < (num_bars - 1) and m.value * bar_agg(i) < m.value * bar_agg(i + 1)):
                rd, i = rd + 1, i + 1
            stat[cls.fields.RD.value] = rd 
            stat[cls.fields.RC.value] = 100 * m.value * (bar_agg(i) - bar_agg(i - rd)) / bar_agg(i)

            # Save point and continue to next iteration
            stat_pts.append(stat)
            d = i
        
        return pd.DataFrame(stat_pts).reset_index(drop=True)


class RSI(TechInd):

    class fields(Enum):
        AVG_UP = "average up"
        AVG_DOWN = "average down"
        CHANGE = "change"
        VAL = "RSI"

    @classmethod
    def compute(cls, bars: pd.DataFrame, barAgg: BarAgg,
        period: int,
    ) -> pd.DataFrame:

        """ 
        Computes the RSI values for a given bar dataframe

        Parameters:
            bars: source bars dataframe
            period: RSI computation period (in bars)
            barAgg: desired aggregator function (e.g. bar's close)
        
        Returns:
            Dataframe containing RSI values and computation details
        """

        bar_agg = lambda i: barAgg(bars.loc[i]) # shorthand accessor
        RSI_pts: List[pd.Series] = []

        for d in range(len(bars)):
            RSI_data = pd.Series(index = cls.index())
            RSI_data[cls.fields.CHANGE.value] = change = bar_agg(d) - bar_agg(d - 1) if d > 0 else 0

            # Compute starting value 
            if (d == period):
                changes  = [RSI_pts[i][cls.fields.CHANGE.value] for i in range(period)]
                tot_up   = sum(change for change in changes if change > 0)
                tot_down = sum(-change for change in changes if change < 0)  # Ensure changes are positive
                RSI_data[cls.fields.AVG_UP.value], RSI_data[cls.fields.AVG_DOWN.value] = avg_up, avg_down = tot_up/period, tot_down/period
            
            # Compute post-starting value recursively
            elif (d > period):
                RSI_data[cls.fields.AVG_UP.value]   = avg_up   = ((period - 1) * RSI_pts[d - 1][cls.fields.AVG_UP.value]   + (change > 0) * change) / period
                RSI_data[cls.fields.AVG_DOWN.value] = avg_down = ((period - 1) * RSI_pts[d - 1][cls.fields.AVG_DOWN.value] - (change < 0) * change) / period
            
            # Safe division check
            RSI_data[cls.fields.VAL.value] = 0 if (d < period) else 100 if avg_down == 0 else 100 - (100 / (1 + avg_up / avg_down))
            RSI_pts.append(RSI_data)

        return pd.DataFrame(RSI_pts).set_index(bars.index)


class BollBands(TechInd):

    class fields(Enum):
        BOLL_MID    = "middle band"
        BOLL_UPPER  = "upper band"
        BOLL_LOWER  = "lower band"
    
    class crossDir(Enum):
        UP   = 1
        DOWN = -1 

    @classmethod
    def compute(cls, bars: pd.DataFrame, barAgg: BarAgg,
        period: int,
        mult: float,
    ) -> pd.DataFrame:

        """ 
        Computes the Bollinger Bands middle, upper, lower values for a given bar dataframe

        Parameters:
            bars: source bars dataframe
            period: moving average and and standard deviation computation period (in bars)
            mult: multiplier for the bands determining distance between upper and lower band (usually 2)
            barAgg: desired aggregator function (e.g. bar's close)
        
        Returns:
            Dataframe containing Bollinger Bands middle, upper, lower values
        """

        bar_agg = lambda i: barAgg(bars.loc[i]) # shorthand accessor
        BB_pts: List[pd.Series] = []

        for d in range(len(bars)):
            # Band calculations
            indices = range(d, max(0, d - period + 1) - 1, -1)
            bar_values: List[float] = [bar_agg(i) for i in indices]
            (per_mean, per_stdev) = (mean(bar_values), stdev(bar_values)) if len(bar_values) >= 2 else (bar_agg(d), 0.0)
            BB_data = pd.Series(index = cls.index())
            BB_data[cls.fields.BOLL_MID.value]   = per_mean
            BB_data[cls.fields.BOLL_UPPER.value] = per_mean + mult * per_stdev
            BB_data[cls.fields.BOLL_LOWER.value] = per_mean - mult * per_stdev

            # Record bands
            BB_pts.append(BB_data)
        return pd.DataFrame(BB_pts).set_index(bars.index)
    
    @classmethod
    def cross(cls, bars: pd.DataFrame, barAgg: BarAgg,
        BollBands: pd.DataFrame,
        index: int,
        band: fields,
        dir: crossDir,
    ) -> bool:
        """ 
        Determines whether a given bar was crossed either up or down

        Parameters:
            bars: source bars dataframe
            BollBands: Bollinger Bands dataframe resulting from calling compute() on this class
            barAgg: bar value accessor (e.g. bar's close)
            index: bar index at which we want to verify crossing (indexes of bars and BollBands aligned)
            band: upper, lower, or middle
            dir: up, or down
        
        Returns:
            True iff crossing of given band in given direction occured
        """
        bar_agg = lambda i: barAgg(bars.loc[i]) # shorthand accessor
        boll_agg = lambda i, band: BollBands.loc[i][band.value]

        return (
            index > 0 and index < len(bars) and
            dir.value * bar_agg(index)     > dir.value * boll_agg(index, band) and 
            dir.value * bar_agg(index - 1) < dir.value * boll_agg(index - 1, band)
        )
    


""" Composite Indicators """

class Div(TechInd):

    class fields(Enum):
        LBP = "left bar index"
        RBP = "right bar index"
        M   = "on max(-1)/min(1)"
        DEG = "degree"
        NEG = "Neg on (-1 if not)"
    
    @classmethod
    def compute(cls, bars: pd.DataFrame, barAgg: BarAgg,
        RSI_period: int,
        max_div_period: int,
        max_neg_period: int,
    ) -> pd.DataFrame:
        """ 
        Computes divergences for both maximums and minimums and concatenates sorted results
        """
        div_res: List[pd.DataFrame] = []
        for loc_stat_type in LocStat.typ:
            div_res.append(
                cls._compute_internal(bars=bars,barAgg= barAgg,
                    locStat=LocStat.compute(bars=bars, m=loc_stat_type, barAgg=barAgg),
                    m=loc_stat_type,
                    RSI_ind=RSI.compute(bars=bars, period=RSI_period, barAgg=barAgg,),
                    max_div_period=max_div_period,
                    max_neg_period=max_neg_period,
                )
            )
        return pd.concat(div_res).sort_values(by=cls.fields.LBP.value, ascending=True).reset_index(drop=True)

    @classmethod
    def _compute_internal(cls, bars: pd.DataFrame, barAgg: BarAgg,
        locStat: pd.DataFrame,
        RSI_ind: pd.DataFrame,
        m: LocStat.typ,
        max_div_period: int,
        max_neg_period: int,
        dbg_ctr: Counter | None = None,
        # mult_div: bool,
        # mult_neg: bool,
    ) -> pd.DataFrame:

        """ 
        Computes Divergences for a given Bars dataframe, on either maximums or minimums

        Parameters:
            bars: source bars dataframe
            barAgg: desired aggregator function (e.g. bar's close)
            locStat: dataframe containing maximum/minimum data (index not aligned with bars index)
            m: Indicates whether we are being passed a maximum or a minimum dataframe
            RSI: dataframe containing RSI data (index aligned with bars index)
            max_div_period: maximum accepted period between LBI and RBI
            max_neg_period: maximum accepted period between RBI and negating stationary point
            dbg_ctr (optional): debugging counter for neg divs (caller responsible for initialization and reset)
            # mult_div: True if we consider multiple divergences with the same right index or left index, False o/w
            # mult_neg: True if we consider multiple divergences with neg at same point, False o/w
        
        Returns:
            Div fields structured dataframe containing divergence information
        """

        # Helper functions
        bar_agg  = lambda i: barAgg(bars.loc[i])
        RSI_val  = lambda i: RSI_ind.loc[i][RSI.fields.VAL.value]
        is_stat  = lambda i: i in locStat[LocStat.fields.BI.value].values
        in_divs  = lambda i: [pdp for pdp in div_pts if (pdp[cls.fields.RBP.value] == i)]
        out_divs = lambda i: [pdp for pdp in div_pts if (pdp[cls.fields.LBP.value] == i)]
        is_new_stat_within = lambda rang, i, d : all(m.value * bar_agg(d) < m.value * bar_agg(j) for j in rang if j >= i) # WAS: lambda i, rang: all(m.value * bar_agg(i) < m.value * bar_agg(j) for j in rang if j > i)

        def collect_root_lbps(source_rbp: str) -> List[str]:
            """ Returns LBP for each fundamental deg. 1 divergence """
            in_lbp = [in_div[cls.fields.LBP.value] for in_div in in_divs(source_rbp)]
            if not in_lbp: # Base case: if source_rbp does not stem into other incoming divergences, return this as a root
                return [source_rbp]
            return [ldp for ldp_list in (collect_root_lbps(lbp) for lbp in in_lbp) for ldp in ldp_list]

        def collect_successor_lbps(source_lbp: str) -> List[str]:
            """  Returns LBP for each div. stemming and including the div. with source_lbp """
            out_lbp = [out_div[cls.fields.LBP.value] for out_div in out_divs(source_lbp)]
            for out_div in out_divs(source_lbp):
                out_lbp += collect_successor_lbps(out_div[cls.fields.RBP.value])
            return out_lbp

        """ 
        Main routine start
        """

        div_pts: List[pd.Series] = []
        stat_indexes: List[int] = [i for i in range(len(bars)) if is_stat(i)]

        # Lagging indicator: we can only declare statPoints, hence Divergences, up to one bar before current
        for d in stat_indexes:

            # Define stationary indexes contained within div and neg max periods, excluding d
            div_stat_indexes = [i for i in range(max(0, d - max_div_period), d) if i in stat_indexes]  # if mult_div else [i for i in stat_indexes if i in div_interval][-1:]
            neg_stat_indexes = [i for i in range(max(0, d - max_neg_period), d) if i in stat_indexes]  # if mult_neg else [i for i in stat_indexes if i in neg_interval][-1:]

            # Iterate throughout all candidate LBI for divergences and div RBI for negs, simultaneously
            for i in max(div_stat_indexes, neg_stat_indexes, key=len):

                # Divergence condition for i as LBI, and d as RBI
                if (i in div_stat_indexes and 
                    is_new_stat_within(div_stat_indexes, i, d) and
                    m.value * RSI_val(d) > m.value * RSI_val(i)
                ):
                    div_pt = pd.Series({
                        cls.fields.LBP.value:   i,
                        cls.fields.RBP.value:   d,
                        cls.fields.M.value:     m.value,
                        cls.fields.DEG.value:   1 + max(
                            (pdp[cls.fields.DEG.value] for pdp in in_divs(i)), 
                            default=0
                        ),
                        cls.fields.NEG.value:   -1  # Means not neg yet
                    }, index=cls.index())
                    div_pts.append(div_pt) # record new divergence

                # NegDiv condition for i as RBI and d as Neg point
                if (i in neg_stat_indexes and 
                    is_new_stat_within(neg_stat_indexes, i, d) and
                    in_divs(i)
                ):
                    # Check neg condition for each root divergence found, if any
                    for root_lbp in collect_root_lbps(source_rbp = i):

                        # Neg condition: RSI at new maximum surpasses value at root divergence bar
                        if (m.value * RSI_val(d) > m.value * RSI_val(root_lbp)): continue

                        # Neg at d the root and all its successors
                        succ_lbps : List[int] = collect_successor_lbps(source_lbp =root_lbp)
                        for succ_div in (pdp for pdp in div_pts if pdp[cls.fields.LBP.value] in succ_lbps):

                            # Debugging: mark all points at which negation recorded
                            if dbg_ctr is not None: dbg_ctr[(succ_div[cls.fields.LBP.value], succ_div[cls.fields.RBP.value], d)] += 1
                            # Maintain earliest recoded divergence flag
                            succ_div[cls.fields.NEG.value] = min(succ_div[cls.fields.NEG.value], d) if succ_div[cls.fields.NEG.value] != -1 else d
                            

        # Return divergence collection (not aligned with original bars dataframe)
        return pd.DataFrame(div_pts).reset_index(drop=True)
    

class Ladders(TechInd):

    class fields(Enum):
        LEFT_STAT   = "left maximum/minimum"
        RIGHT_STAT  = "right maximum/minimum"
        STAT_PTS    = "total contained maximums and minimums"
        DIRECTION   = "upward(-1) / downward(1)"
    
    class dir(Enum):
        UP   = -1
        DOWN = 1
    
    @classmethod
    def compute(cls, bars: pd.DataFrame, barAgg: BarAgg,
        min_stat_pts: int,
        max_stat_pts: int = 2**31 - 1,
    ) -> pd.DataFrame:
        """ Computes both upward and downward ladders and sorts them by left max/min"""
        res_ladders: List[pd.DataFrame] = []
        for dir_type in cls.dir:
            res_ladders.append(
                cls._compute_internal(bars=bars,barAgg= barAgg,
                    max_ind=LocStat.compute(bars=bars, m=LocStat.typ.MAX, barAgg=barAgg),
                    min_ind=LocStat.compute(bars=bars, m=LocStat.typ.MIN, barAgg=barAgg),
                    m=dir_type,
                    min_stat_pts=min_stat_pts,
                    max_stat_pts=max_stat_pts
                )
            )
        return pd.concat(res_ladders).sort_values(by=cls.fields.LEFT_STAT.value, ascending=True).reset_index(drop=True)

    @classmethod
    def _compute_internal(cls, bars: pd.DataFrame, barAgg: BarAgg,
        max_ind: pd.DataFrame,
        min_ind: pd.DataFrame,
        m: dir,
        min_stat_pts: int,
        max_stat_pts: int,
    ) -> pd.DataFrame:

        """ 
        Computes "Ladders", upward/downward trends determined by maximums and minimums rising/falling simulataneously

        Parameters:
            bars: source bars dataframe
            max_ind, min_ind: Computed maximum and minimum indicators
            m: whether we want upward or downward ladders  
            barAgg: desired aggregator function (e.g. bar's close)
            min_stat_pts, max_stat_pts: minimum and maximum contiguous stationary points (includes both maximums and minimums) that a ladder has to contain
        
        Returns:
            Dataframe containing ladders left and right stationary points, as well as direction
        
        NOTE:
            If UP ladder:   starts at MIN, ends at any
            If DOWN ladder: starts at MAX, ends at any
        """

        # Ensure iteration lists aligned and ending simultaneously
        min_len: int = min(len(min_ind), len(max_ind))
        min_index_list = min_ind[LocStat.fields.BI.value].values.tolist()[-min_len:]
        max_index_list = max_ind[LocStat.fields.BI.value].values.tolist()[-min_len:]
        ladder_start_indexes = min_index_list if m == cls.dir.UP else max_index_list
        ladders: List[pd.Series] = []

        # Define helper functions
        bar_agg             = lambda i: barAgg(bars.loc[i]) # shorthand accessor
        first_in_start      = lambda i, j: i if i in ladder_start_indexes else j
        stat_points_within  = lambda l, r: sum(i in max_index_list or i in min_index_list for i in range(l, r + 1))
        save_ladder         = lambda l, r: ladders.append(
            pd.Series({
                cls.fields.LEFT_STAT.value:  int(l),
                cls.fields.RIGHT_STAT.value: int(r),
                cls.fields.STAT_PTS.value:   stat_points_within(int(l), int(r)),
                cls.fields.DIRECTION.value:  m.value,
            })
        )

        # Initialize iteration variables
        max_it, min_it = iter(max_index_list), iter(min_index_list)
        prev_max, prev_min = 0, 0 
        curr_max, curr_min = next(max_it), next(min_it)
        right = left = min(curr_max, curr_min)

        # Start bar iteration
        while curr_max < max_index_list[-1] and curr_min < min_index_list[-1]:

            # Advance to next max-min couple
            prev_max, prev_min = curr_max, curr_min 
            curr_max, curr_min = next(max_it), next(min_it)

            # For each max-min couple select leftmost and rightmost
            prev_left, prev_right = min(prev_min, prev_max), max(prev_min, prev_max)
            curr_left, curr_right = min(curr_min, curr_max), max(curr_min, curr_max)

            # Check ladder condition on each set of left/right maximums/minimums
            results: List[bool] = []
            for (prev, curr) in [(prev_left, curr_left), (prev_right, curr_right)]:

                # Main ladder condition
                results.append(m.value * bar_agg(prev) > m.value * bar_agg(curr))

                # Maintain right to last consecutive success
                right = curr if all(results) else right

            # If not both conditions passed, save ladder and reassign ladder pointers to first satisfactory point for trend initiation
            if not all(results):
                save_ladder(left, right)
                res_left, res_right = results
                right = left = first_in_start(curr_left, curr_right) if not res_right else first_in_start(prev_right, curr_left) # if not res_left
        
        # Iteration termination
        if left < right: save_ladder(left, right) # record final ladder if progress was made
        select_ladders = [ladder for ladder in ladders if ladder[cls.fields.STAT_PTS.value] >= min_stat_pts and ladder[cls.fields.STAT_PTS.value] <= max_stat_pts]
        return pd.DataFrame(select_ladders).reset_index(drop=True)
    









