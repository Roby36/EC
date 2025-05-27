
from enum import Enum

status_file: str = "ClientWebConnectionStatus.log"
#tickle_running: bool = True
stdDateFormat: str = '%Y%m%d-%H:%M:%S'

class AccountID(Enum):
    FABIO: str = "DUA094996"

UNIX_TIME_UNITS: str = "ms"
BAR_DATA_ENTRY: str = "data"
LIVE_ORDERS_ENTRY: str = "orders"

""" 
Bar size settings:
Daily:
    period: 1 YEAR; bar: 1 DAY
    ~ 360 data points
Hourly:
    period: 1 MONTH; bar: 1 HOUR
    ~ 250 30-minute data points
"""
    
