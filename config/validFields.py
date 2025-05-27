
from enum import Enum

""" Market data request fields"""
class PeriodUnits(Enum):
    SECOND = 'S'
    DAY = 'd'
    WEEK = 'w'
    MONTH = 'm'
    YEAR = 'y'

class BarUnits(Enum):
    SECOND = "secs"
    MINUTE = "mins"
    HOUR = "hrs"
    DAY = "d"
    WEEK = "w"
    MONTH = "m"

class BarType(Enum):
    LAST = "last"
    BID = "bid"
    ASK = "ask"
    MIDPOINT = "midpoint"
    FEERATE = "FeeRate"
    INVENTORY = "inventory"

class PeriodDirecion(Enum):
    START_TO_NOW = -1
    NOW_TO_START = 1


""" Order requests fields """
class Tif(Enum):
    GTC = "GTC"
    OPG = "OPG"
    DAY = "DAY"
    IOC = "IOC"
    PAX = "PAX"

class OrderTypes(Enum):
    MARKET = "MKT"
    LIMIT = "LMT"
    STOP = "STP"
    STOP_LIMIT = "STOP_LIMIT"
    MIDPRICE = "MIDPRICE"
    TRAIL="TRAIL"
    TRAIL_LIMIT="TRAILLMT"

class OrderStatus(Enum):
    INACTIVE = "Inactive"
    PENDING_SUBMIT = "PendingSubmit"
    PRE_SUBMITTED = "PreSubmitted"
    SUBMITTED = "Submitted"
    FILLED = "Filled"
    PENDING_CANCEL = "PendingCancel"
    CANCELLED = "Cancelled"
    WARN_STATE = "WarnState"
    SORT_BY_TIME = "SortByTime"

class OrderSide(Enum):
    BUY = "BUY"
    SELL = "SELL"

class OrderFields(Enum):
    # Return Fields
    ACCT = "acct"
    CONIDEX = "conidex"
    CONID = "conid"
    ORDERID = "orderId"
    CASHCCY = "cashCcy"
    SIZE_AND_FILLS = "sizeAndFills"
    ORDER_DESC = "orderDesc"
    DESCRIPTION1 = "description1"
    TICKER = "ticker"
    SECTYPE = "secType"
    LISTING_EXCHANGE = "listingExchange"
    REMAINING_QUANTITY = "remainingQuantity"
    FILLED_QUANTITY = "filledQuantity"
    COMPANY_NAME = "companyName"
    STATUS = "status"
    ORDER_CCP_STATUS = "order_ccp_status"
    ORIG_ORDER_TYPE = "origOrderType"
    SUPPORTS_TAX_OPT = "supportsTaxOpt"
    LAST_EXECUTION_TIME = "lastExecutionTime"
    ORDER_TYPE = "orderType"
    BG_COLOR = "bgColor"
    FG_COLOR = "fgColor"
    ORDER_REF = "order_ref"
    TIME_IN_FORCE = "timeInForce"
    LAST_EXECUTION_TIME_R = "lastExecutionTime_r"
    SIDE = "side"
    AVG_PRICE = "avgPrice"
    TOTAL_SIZE = "totalSize"

    # Request fields not already in Response fields
    PARENT_ID = "parentId"
    PRICE = "price"
    COID = "cOID"
    QUANTITY = "quantity"
    TIF = "tif"
    ACCT_ID = "acctId"



