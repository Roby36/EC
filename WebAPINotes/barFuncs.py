
from typing import List
import pandas as pd

from config.validFields import *
from config import globals
from . import cpwalib


class Bars:
    """ 
    This class handles data extraction from IBKR Web API and 
    organizes it into dataframe that can then be directly processed by the Indicators module
    """

    class fields(Enum):
        OPEN = "open"
        CLOSE = "close"
        HIGH = "high"
        LOW = "low"
        VOLUME = "volume"
        TIME = "time"
        DATE = "date"

    # Bar labels coming from marketdata_history endpoint
    mkhBarKeys: dict = {
        "o": fields.OPEN.value,
        "c": fields.CLOSE.value,
        "h": fields.HIGH.value,
        "l": fields.LOW.value,
        "t": fields.TIME.value,
        "v": fields.VOLUME.value
    }

    @classmethod
    def reqBars(cls,
        conid: str,
        periods: int,
        period_unit: PeriodUnits,
        bar: int,
        bar_unit: BarUnits,
        outsideRth: bool,
    # optionals
        startTime: pd.Timestamp | None = None,
        exchange: str | None = None,
    ) -> pd.DataFrame:
        """ 
        Returns a clean bars dataframe, handling marketdata/history endpoint request procedure gracefully.

        Parameters(see marketDataHistory params)

        Assumptions:
            (3) bin/run.sh root/conf.yaml keeps port 5000 open, else will crash
            (5) Response object is valid .json upon successful request

        Frequently used bar size settings:
            Daily:
                period: 1 YEAR; bar: 1 DAY
                ~ 360 data points
            Hourly:
                period: 1 MONTH; bar: 1 HOUR
                ~ 250 30-minute data points
        """

        # Gracefully handle marketdata request and extract corresponding data
        req_mdh = cpwalib.marketDataHistory(
            conid=conid, period=periods, period_unit=period_unit, bar=bar, bar_unit=bar_unit, outsideRth=outsideRth, startTime=startTime, exchange=exchange
        )
        resp: bool = cpwalib.checkResponse("", req_mdh)
        mdh_json: dict = req_mdh.json() if resp else {}
        mdh_data: List[dict] = mdh_json.get(globals.BAR_DATA_ENTRY, [])

        # Clean resulting dataframe
        mdh = pd.DataFrame(mdh_data).rename(columns=cls.mkhBarKeys)
        mdh[cls.fields.DATE.value] = mdh[cls.fields.TIME.value].apply(lambda t: pd.Timestamp(t, unit=globals.UNIX_TIME_UNITS)) if cls.fields.TIME.value in mdh.columns else None
        mdh = mdh.sort_values(by=cls.fields.TIME.value, ascending=True) if cls.fields.TIME.value in mdh.columns else mdh

        return mdh


    @classmethod
    def aggBars(cls,
        mdh: pd.DataFrame,
        agg_fact: int,
        bar_size: pd.Timedelta,
    ) -> pd.DataFrame:
        """ 
        Aggregates granular bar object (e.g. 30 mins) into less granular (e.g. 1 hour)

        Parameters:
            mdh: source bars dataframe (fully processed)
            agg_fact: granularity ratio between output and input bars 
            bar_size: bar granularity of input dataframe
        
        Returns:
            Less granular bar df by agg_fact, with same columns as source bar df
        
        Assumptions:
            (1) Fully-fledged pre-processed dataframe with all the columns initialized as required
        """

        i: int = 0
        num_rows : int  = mdh.shape[0]
        agg_mdh_rows = []

        while (i < num_rows):

            # Iterate until (1) bars are adjacent and (2) within aggregating factor (3) within bounds
            j: int = i
            while j < (num_rows - 1) and (j - i < agg_fact - 1):
                curr = pd.Timestamp(mdh.loc[j,      cls.fields.TIME.value], unit=globals.UNIX_TIME_UNITS)
                next = pd.Timestamp(mdh.loc[j + 1 , cls.fields.TIME.value], unit=globals.UNIX_TIME_UNITS)
                diff: pd.Timedelta = next - curr
                if (diff != bar_size):
                    break
                j = j + 1
            
            # Initialize agg_bar with default values or data types
            start_bar, end_bar, agg_bar     = mdh.loc[i], mdh.loc[j], mdh.loc[0].copy() # initialize to some default value with same index to avoid warnings

            # Aggregate data
            agg_bar.update({field: start_bar[field] for field in [cls.fields.OPEN.value, cls.fields.TIME.value, cls.fields.DATE.value]})
            agg_bar.update({field: end_bar[field] for field in [cls.fields.CLOSE.value]})
            agg_bar[cls.fields.HIGH.value]   = mdh.loc[i : j, ][cls.fields.HIGH.value].max()
            agg_bar[cls.fields.LOW.value]    = mdh.loc[i : j, ][cls.fields.LOW.value].min()
            agg_bar[cls.fields.VOLUME.value] = mdh.loc[i : j, ][cls.fields.VOLUME.value].sum()

            # Store Series in a list
            agg_mdh_rows.append(agg_bar)

            # Update i to the next starting point
            i = j + 1  

        # Create DataFrame from the list of Series
        agg_mdh = pd.DataFrame(agg_mdh_rows)

        # Reset the index of the DataFrame
        agg_mdh.reset_index(drop=True, inplace=True)

        return agg_mdh




