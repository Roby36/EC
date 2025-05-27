
import time
import requests
import copy
import logging
from typing import List
import pandas as pd

from config import globals
from . import endpoints
from config.validFields import *

def checkResponse(url: str, response: requests.Response) -> bool:
    resp_code = response.status_code 
    if resp_code < 200 or resp_code >= 300:
        logging.warning(f"Request to {url} was unsuccessful. Response code: {resp_code}")
        return False
    else: 
        logging.info(f"Request to {url} returned successfully. Response code: {resp_code}")
        return True




""" 
    Wrappers: Will go in wrappers.py dedicated file
"""

def contractSearch(req_json: dict) -> requests.Response:
    """ 
    REQUEST OBJECT:

        "symbol": String. Required
        Underlying symbol of interest. May also pass company name if ‘name’ is set to true, or bond issuer type to retrieve bonds.

        "secType": String.
        Valid Values: “STK”, “IND”, “BOND”
        Declares underlying security type.

        "name": bool.
        Determines if symbol reflects company name or ticker symbol.

    
    RESPONSE OBJECT:

        “conid”: String.
        Conid of the given contract.

        “companyHeader”: String.
        Extended company name and primary exchange.

        “companyName”: String.
        Name of the company.

        “symbol”: String.
        Company ticker symbol.

        “description”: String.
        Primary exchange of the contract.

        “restricted”: bool.
        Returns if the contract is available for trading.

        “fop”: String.
        Returns a string of dates, separated by semicolons.
        Value Format: “YYYYMMDD;YYYYMMDD;YYYYMMDD”

        “opt”: String.
        Returns a string of dates, separated by semicolons.
        Value Format: “YYYYMMDD;YYYYMMDD;YYYYMMDD”

        “war”: String.
        Returns a string of dates, separated by semicolons.
        Value Format: “YYYYMMDD;YYYYMMDD;YYYYMMDD”

        “sections”: Array of objects

            “secType”: String.
            Given contracts security type.

            “months”: String.
            Returns a string of dates, separated by semicolons.
            Value Format: “JANYY;FEBYY;MARYY”

            “symbol”: String.
            Symbol of the instrument.

            “exchange”: String.
            Returns a string of exchanges, separated by semicolons.
            Value Format: “EXCH;EXCH;EXCH”

        
    UNIQUE FOR BONDS:
        “issuers”: Array of objects
        Array of objects containing the id and name for each bond issuer.

        “id”: String.
        Issuer Id for the given contract.

        “name”: String.
        Name of the issuer.

        “bondid”: int.
        Bond type identifier.

        “conid”: String.
        Contract ID for the given bond.

        “companyHeader”: String.
        Name of the bond type
        Value Format: “Corporate Fixed Income”

        “companyName”: null
        Returns ‘null’ for bond contracts.

        “symbol”:null
        Returns ‘null’ for bond contracts.

        “description”:null
        Returns ‘null’ for bond contracts.

        “restricted”:null
        Returns ‘null’ for bond contracts.

        “fop”:null
        Returns ‘null’ for bond contracts.

        “opt”:null
        Returns ‘null’ for bond contracts.

        “war”:null
        Returns ‘null’ for bond contracts.

        “sections”: Array of objects
        Only relays “secType”:”BOND” in the Bonds section.
    """
    req_url: str = endpoints.base_url + endpoints.secdef_search
    req_contract = requests.post(url= req_url, verify=False, json=req_json)
    checkResponse(req_url, req_contract)
    return req_contract


def contractInfo(req_json: dict) -> requests.Response:
    """ 
    REQUEST OBJECT:

        conid: String. Required
        Contract identifier of the underlying. May also pass the final derivative conid directly.

        sectype: String. Required
        Security type of the requested contract of interest.

        month: String. Required for Derivatives
        Expiration month for the given derivative.

        exchange: String. Optional
        Designate the exchange you wish to receive information for in relation to the contract.

        strike: String. Required for Options and Futures Options
        Set the strike price for the requested contract details

        right: String. Required for Options
        Set the right for the given contract.
        Value Format: “C” for Call or “P” for Put.

        issuerId: String. Required for Bonds
        Set the issuerId for the given bond issuer type.
        Example Format: “e1234567”


    RESPONSE OBJECT:
        conid: int.
        Contract Identifier of the given contract

        ticker: String
        Ticker symbol for the given contract

        secType: String.
        Security type for the given contract.

        listingExchange: String.
        Primary listing exchange for the given contract.

        exchange: String.
        Exchange requesting data for.

        companyName: String.
        Name of the company for the given contract.

        currency: String
        Traded currency allowed for the given contract.

        validExchanges: String*
        Series of all valid exchanges the contract can be traded on in a single comma-separated string.
        priceRendering: null.

        maturityDate: String
        Date of expiration for the given contract.

        right: String.
        Right (P or C) for the given contract.

        strike: Float.
        Returns the given strike value for the given contract.
    """
    req_url: str = endpoints.base_url + endpoints.secdef_info + "?" + "&".join([f"{key}={item}" for key, item in req_json.items()])
    req_contract_info = requests.get(url = req_url, verify = False)
    checkResponse(req_url, req_contract_info)
    return req_contract_info


def contractStrikes(req_json: dict) -> requests.Response:
    """ 
    Same REQUEST OBJECT as contractInfo
    """
    req_url: str = endpoints.base_url + endpoints.secdef_strikes + "?" + "&".join([f"{key}={item}" for key, item in req_json.items()])
    req_contract_strikes = requests.get(url = req_url, verify = False)
    checkResponse(req_url, req_contract_strikes)
    return req_contract_strikes


def marketDataSnapshot(conids: List[str], fields: List[int])-> requests.Response:
    """ 
    REQUEST OBJECT:

        "conids": String. Required
        Contract identifier for the contract of interest.
        May provide a comma-separated series of contract identifiers.

        "fields": String. Required
        Specify a series of tick values to be returned.
        May provide a comma-separated series of field ids.
        See Market Data Fields for more information.

    RESPONSE OBJECT:

        server_id: String.
        Returns the request’s identifier.

        conidEx: String.
        Returns the passed conid field. May include exchange if specified in request.

        conid: int.
        Returns the contract id of the request

        _updated: int*.
        Returns the epoch time of the update in a 13 character integer .

        6119: String.
        Field value of the server_id. Returns the request’s identifier.

        fields*: String.
        Returns a response for each request. Some fields not be as readily available as others. See the Market Data section for more details.

        6509: String.
        Returns a multi-character value representing the Market Data Availability.
    """
    conids_str: str = "conids=" + ",".join([cid for cid in conids])
    fields_str: str = "fields=" + ",".join([str(field) for field in fields])
    params: str     = "&".join([conids_str, fields_str])
    req_url: str    = endpoints.base_url + endpoints.marketdata_snapshot + "?" + params
    req_mds         = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_mds)
    return req_mds


def historicalData(
    conid: str,
    period: int,
    period_unit: PeriodUnits,
    bar: int,
    bar_unit: BarUnits,
    outsideRth: bool = False,
    startTime: pd.Timestamp | None = None,
    direction: PeriodDirecion = PeriodDirecion.START_TO_NOW,
    barType: BarType = BarType.MIDPOINT
) -> requests.Response:
    """ 
    REQUEST OBJECT: 
        conid: String. Required
        The contract identifier for which data should be requested.

        period: String. Required
        The duration for which data should be requested.
        Available Values: See HMDS Period Units

        bar: String. Required
        The bar size for which bars should be returned.
        Available Values: See HMDS Bar Sizes

        outsideRth: bool.
        Define if data should be returned for trades outside regular trading hours.

        startTime: String.
        Specify the value from where historical data should be taken.
        Value Format: UTC; YYYYMMDD-HH:mm:dd
        Defaults to the current date and time

        direction: String.
        Specify the direction from which market data should be returned
        Available Values: -1: time from the startTime to now; 1: time from now to the end of the period.
        Defaults to 1

        barType: String.
        Returns valid bar types for which data may be requested.
        Available Values: Last, Bid, Ask, Midpoint, FeeRate, Inventory
        Defaults to Last for Stocks, Options, Futures, and Futures Options.


    INITIAL RESPONSE OBJECT
        The first time a user makes a request to the /hmds/history endpoints will result in a 404 error. This initial request instantiates the historical market data services allowing future requests to return data. Subsequent requests will return data as expected.
        <html><body><h1>Resource not found</h1></body></html>
 

    RESPONSE OBJECT
        startTime: String.
        Returns the initial time of the historical data request.
        Returned in UTC formatted as YYYYMMDD-HH:mm:ss

        startTimeVal: int.
        Returns the initial time of the historical data request.
        Returned in epoch time.

        endTime: String.
        Returns the end time of the historical data request.
        Returned in UTC formatted as YYYYMMDD-HH:mm:ss

        endTimeVal: int.
        Returns the end time of the historical data request.
        Returned in epoch time.

        data: Array of objects.
        Returns all historical bars for the requested period.
        [{
            t: int.
            Returns the epoch timestamp of the bar.

            o: float.
            Returns the Open value of the bar.

            c: float.
            Returns the Close value of the bar.

            h: float.
            Returns the High value of the bar.

            l: float.
            Returns the Low value of the bar.

            v: float.
            Returns the Volume of the bar.
        }],

        points: int.
        Returns the total number of data points in the bar.

        mktDataDelay: int.
        Returns the amount of delay, in milliseconds, to process the historical data request.
    """

    params = "&".join([
        f"conid={conid}", 
        f"period={period}{period_unit.value}", 
        f"bar={bar}{bar_unit.value}",
        f"outsideRth={outsideRth}", 
        f"startTime={startTime.strftime(globals.stdDateFormat)}" if startTime is not None else "",
        f"direction={direction.value}",
        f"barType={barType.value}"
    ])
    req_url = endpoints.base_url + endpoints.hmds_history + "?" + params
    req_hd = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_hd)
    return req_hd


def marketDataHistory(
    conid: str,
    period: int,
    period_unit: PeriodUnits,
    bar: int,
    bar_unit: BarUnits,
    outsideRth: bool,
# optionals
    startTime: pd.Timestamp | None = None,
    exchange: str | None = None,
) -> requests.Response:
    """ 
    REQUETS OBJECT 
        conid: String. Required
        Contract identifier for the ticker symbol of interest.

        bar: String. Required
        Individual bars of data to be returned.
        Possible value– 1min, 2min, 3min, 5min, 10min, 15min, 30min, 1h, 2h, 3h, 4h, 8h, 1d, 1w, 1m

        period: String.
        Overall duration for which data should be returned.
        Default to 1w
        Available time period– {1-30}min, {1-8}h, {1-1000}d, {1-792}w, {1-182}m, {1-15}y

        startTime: String
        Starting date of the request duration.

        exchange: String.
        Returns the exchange you want to receive data from.

        outsideRth: bool.
        Determine if you want data after regular trading hours.


    RESPONSE OBJECT 
        serverId: String.
        Internal request identifier.

        symbol: String.
        Returns the ticker symbol of the contract.

        text: String.
        Returns the long name of the ticker symbol.

        priceFactor: String.
        Returns the price increment obtained from the display rules.

        startTime: String.
        Returns the initial time of the historical data request.
        Returned in UTC formatted as YYYYMMDD-HH:mm:ss

        high: String.
        Returns the High values during this time series with format %h/%v/%t.
            %h is the high price (scaled by priceFactor),
            %v is volume (volume factor will always be 100 (reported volume = actual volume/100))
            %t is minutes from start time of the chart

        low: String.
            Returns the low value during this time series with format %l/%v/%t.
            %l is the low price (scaled by priceFactor),
            %v is volume (volume factor will always be 100 (reported volume = actual volume/100))
            %t is minutes from start time of the chart

        timePeriod: String.
        Returns the duration for the historical data request

        barLength: int.
        Returns the number of seconds in a bar.

        mdAvailability: String.
        Returns the Market Data Availability.
        See the Market Data Availability section for more details.

        mktDataDelay: int.
        Returns the amount of delay, in milliseconds, to process the historical data request.

        outsideRth: bool.
        Defines if the market data returned was inside regular trading hours or not.

        volumeFactor: int.
        Returns the factor the volume is multiplied by.

        priceDisplayRule: int.
        Presents the price display rule used.
        For internal use only.

        priceDisplayValue: String.
        Presents the price display rule used.
        For internal use only.

        negativeCapable: bool.
        Returns whether or not the data can return negative values.

        messageVersion: int.
        Internal use only.

        data: Array of objects.
        Returns all historical bars for the requested period.
            [{
            o: float.
            Returns the Open value of the bar.

            c: float.
            Returns the Close value of the bar.

            h: float.
            Returns the High value of the bar.

            l: float.
            Returns the Low value of the bar.

            v: float.
            Returns the Volume of the bar.

            t: int.
            Returns the Operator Timezone Epoch Unix Timestamp of the bar.
            }],

        points: int.
        Returns the total number of data points in the bar.

        travelTime: int.
        Returns the amount of time to return the details.
    """

    params = "&".join([
        f"conid={conid}", 
        f"period={period}{period_unit.value}", 
        f"bar={bar}{bar_unit.value}",
        f"outsideRth={outsideRth}", 
        f"startTime={startTime.strftime(globals.stdDateFormat)}" if startTime is not None else "",
        f"exchange={exchange}" if exchange is not None else ""
    ])
    req_url = endpoints.base_url + endpoints.mkt_data_history + "?" + params
    req_mdh = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_mdh)
    return req_mdh

def placeOrder(accountID: str, orders: List[dict]) -> requests.Response:
    """ 
    REQUEST OBJECT
        accountId: String.
        The account ID for which account should place the order.
        Financial Advisors may specify

        orders: Array of Objects. Required
        Used to the order content.
            [{
            acctId: String.
            It should be one of the accounts returned by /iserver/accounts.
            If not passed, the first one in the list is selected.

            conid: int. Required*
            conid is the identifier of the security you want to trade
            You can find the conid with /iserver/secdef/search.
            *Can use conidex instead of conid.

            conidex: String. Optional*
            A mix of the contract identifier and exchange
            *Can be used instead of conid when specifying the contract identifier of a security.
            Value format: “conid@exchange”

            secType: String.
            The contract-identifier (conid) and security type (type) specified as a concatenated value
            Value Format: “conid:type”

            cOID: String.
            Customer Order ID.
            An arbitrary string that can be used to identify the order
            The value must be unique for a 24h span.
            Do not set this value for child orders when placing a bracket order.

            parentId: String.
            Only specify for child orders when placing bracket orders.
            The parentId for the child order(s) must be equal to the cOId (customer order id) of the parent.

            orderType: String. Required
            The order-type determines what type of order you want to send.
            Available Order Types: LMT, MKT, STP, STOP_LIMIT, MIDPRICE, TRAIL, TRAILLMT

            listingExchange: String.
            Primary routing exchange for the order.
            By default we use “SMART” routing.
            Possible values are available via the endpoint: /iserver/contract/{conid}/info

            isSingleGroup: bool.
            Set to true if you want to place a single group orders(OCA)

            outsideRTH: bool.
            Set to true if the order can be executed outside regular trading hours.

            price: float. Required for LMT or STOP_LIMIT
            This is typically the limit price.
            For STP|TRAIL this is the stop price.
            For MIDPRICE this is the option price cap.

            auxPrice: float. Required for STOP_LIMIT and TRAILLMT orders.
            Stop price for STOP_LIMIT and TRAILLMT orders.
            You must specify both price and auxPrice for STOP_LIMIT|TRAILLMT orders.

            side: String. Required
            Valid Values: SELL or BUY

            ticker: String.
            This is the underlying symbol for the contract.

            tif: String. Required
            The Time-In-Force determines how long the order remains active on the market.
            Valid Values: GTC, OPG, DAY, IOC, PAX (CRYPTO ONLY).

            trailingAmt: float. Required for TRAIL and TRAILLMT order
            optional if order is TRAIL, or TRAILLMT.
            When trailingType is amt, this is the trailing amount
            When trailingType is %, it means percentage.

            trailingType: String. Required for TRAIL and TRAILLMT order
            This is the trailing type for trailing amount.
            You must specify both trailingType and trailingAmt for TRAIL and TRAILLMT order
            Valid Values: “amt” or “%”

            referrer: String.
            Custom order reference

            quantity: float. Required*
            Used to designate the total number of shares traded for the order.

            cashQty: float.
            Used to specify the monetary value of an order instead of the number of shares.
            When using ‘cashQty’ don’t specify ‘quantity’
            Cash quantity orders are provided on a non-guaranteed basis.
            In addition to the monetary value, the order uses a maximum size that is calculated using the Cash Quantity Estimated Factor, which can be modified in TWS Order Presets.
            Please note this will round the order down to the nearest whole share, which may be 0 depending on the value provided compared to the current share price.

            fxQty: float.
            This is the cash quantity field which can only be used for Currency Conversion Orders.
            When using ‘fxQty’ don’t specify ‘quantity’.

            useAdaptive: boolean
            If true, the system will use the Price Management Algo to submit the order.
            Read more on our Price Management Algo page. https://www.interactivebrokers.com/en/index.php?f=43423

            isCcyConv: boolean
            set to true if the order is a FX conversion order

            allocationMethod: String.
            Set the allocation method when placing an order using an FA account for a group.
            Based on value set in Trader Workstation.

            manualOrderTime: int.
            Only used for Brokers and Advisors. Mark the time to manually record initial order entry.
            Must be sent as epoch time integer.

            deactivated: bool.
            Functions the same as Saving an Order in Trader Workstation.

            strategy: String.
            Specify which IB Algo algorithm to use for this order.

            strategyParameters: Array.
            The IB Algo parameters for the specified algorithm.
            }]


    RESPONSE OBJECT
        orderId: String.
        Returns the orders identifier which can be used for order tracking, modification, and cancellation.

        order_status: String.
        Returns the order status of the current market order.
        See Order Status Value for more information.

        encrypt_message: String.
        Returns a “1” to display that the message sent was encrypted.
    """

    req_url = endpoints.base_url + endpoints.accountID_orders(accountID)
    json_content = {"orders": [order for order in orders]}
    req_po = requests.post(url=req_url, json=json_content, verify=False)
    checkResponse(req_url, req_po)
    return req_po


def cancelOrder(accountID: str, orderID: str) -> requests.Response:
    req_url = endpoints.base_url + endpoints.accountID_orderID(accountID, orderID)
    req_cancel = requests.delete(url = req_url, verify=False)
    checkResponse(req_url, req_cancel)
    return req_cancel

def orderStatus(orderID: str) -> requests.Response:
    req_url = endpoints.base_url + endpoints.order_status(orderID)
    req_os = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_os)
    return req_os

def liveOrders(filters: List[OrderStatus], force: bool, accountID: str = "") -> requests.Response:
    """ 
    REQUEST OBJECT   
        filters: String.
        Optionally filter your list of orders by a unique status value. More than one filter can be passed, separated by commas.
        For available filters, see Order Status Values

        force: bool.
        Force the system to clear saved information and make a fresh request for orders. Submission will appear as a blank array.


    RESPONSE OBJECT    
        orders: Array of objects.
        Contains all orders placed on the account for the day.
            [{
            acct: String.
            Returns the accountID for the submitted order.

            conidex: String.
            Returns the contract identifier for the order.

            conid: int.
            Returns the contract identifier for the order.

            orderId: int.
            Returns the local order identifier of the order.

            cashCcy: String.
            Returns the currency used for the order.

            sizeAndFills: String.
            Returns the size of the order and how much of it has been filled.

            orderDesc: String.
            Returns the description of the order including the side, size, order type, price, and tif.

            description1: String.
            Returns the local symbol of the order.

            ticker: String.
            Returns the ticker symbol for the order.

            secType: String.
            Returns the security type for the order.

            listingExchange: String.
            Returns the primary listing exchange of the orer.

            remainingQuantity: float.
            Returns the remaining size for the order to fill.

            filledQuantity: float.
            Returns the size of the order already filled.

            companyName: String.
            Returns the company long name.

            status: String.
            Returns the current status of the order.

            order_ccp_status: String.
            Returns the current status of the order.

            origOrderType: String.
            Returns the original order type of the order, whether or not the type has been changed.

            supportsTaxOpt: String.
            Returns if the order is supported by the Tax Optimizer.

            lastExecutionTime: String.
            Returns the datetime of the order’s most recent execution.
            Time returned is based on UTC timezone.
            Value Format: YYMMDDHHmmss

            orderType: String.
            Returns the current order type, or the order at the time of execution.

            bgColor: String.
            Internal use only.

            fgColor: String.
            Internal use only.

            order_ref: String.
            User defined string used to identify the order. Value is set using “cOID” field while placing an order.

            timeInForce: String.
            Returns the time in force (tif) of the order.

            lastExecutionTime_r: int.
            Returns the epoch time of the most recent execution on the order.

            side: String.
            Returns the side of the order.

            avgPrice: String.
            Returns the average price of execution for the order.
            }]

        snapshot: bool.
        Returns if the data is a snapshot of the account’s orders.
    """

    filters_str: str = "filters=" + ",".join([filter.value for filter in filters]) if filters else ""
    params: str = "&".join([filters_str, f"force={force}"])
    req_url = endpoints.base_url + endpoints.accountID_orders(accountID) + "?" + params
    req_lo = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_lo)
    return req_lo
 
def orderReply(replyID: str, confirmed: bool) -> requests.Response:
    """
    REQUEST OBJECT
    replyId: String. Required
    Include the id value from the prior order request relating to the particular order’s warning confirmation.

    Body Params
    confirmed: bool. Required
    Pass your confirmation to the reply to allow or cancel the order to go through.
    true will agree to the message transmit the order.
    false will decline the message and discard the order.
    """
    req_url = endpoints.base_url + endpoints.reply(replyID)
    json_body = {"confirmed": confirmed}
    req_or = requests.post(url=req_url, json=json_body, verify=False)
    checkResponse(req_url, req_or)
    return req_or

def suppressMessages(msgs: List[str]) -> requests.Response:
    req_url = endpoints.base_url + endpoints.questions_suppress
    json_body = {"messageIds": msgs}
    req_sm = requests.post(url=req_url, json=json_body, verify=False)
    checkResponse(req_url, req_sm)
    return req_sm

def resetSuppresseMessages() -> requests.Response:
    req_url = endpoints.base_url + endpoints.questions_suppress + "/reset"
    req_rm = requests.post(url=req_url, json={}, verify=False)
    checkResponse(req_url, req_rm)
    return req_rm

def trades(days: int| None = None) -> requests.Response:
    """
    REQUEST OBJECT 

        days: String.
        Specify the number of days to receive executions for, up to a maximum of 7 days.
        If unspecified, only the current day is returned.

        
    RESPONSE OBJECT 
        execution_id: String.
        Returns the execution ID for the trade.

        symbol: String.
        Returns the underlying symbol.

        supports_tax_opt: String.
        Returns whether or not tax optimizer is supported for the order.

        side: String.
        Returns the side of the order, Buy or Sell.

        order_description: String.
        Returns the description of the order including the side, size, symbol, order type, price, and tif.

        order_ref: String.
        User defined string used to identify the order. Value is set using “cOID” field while placing an order.

        trade_time: String.
        Returns the UTC format of the trade time.

        trade_time_r: int.
        Returns the epoch time of the trade.

        size: float.
        Returns the quantity of the order.

        price: String.
        Returns the price of trade execution.

        submitter: String.
        Returns the username that submitted the order.

        exchange: String.
        Returns the exchange the order was executed on.

        commission: String.
        Returns the cost of commission for the trade.

        net_amount: float.
        Returns the total net cost of the order.

        account: String.
        Returns the account identifier.

        accountCode: String.
        Returns the account identifier.

        company_name: String.
        Returns the long name of the contract’s company.

        contract_description_1: String.
        Returns the local symbol of the order.

        sec_type: String.
        Returns the security type of the contract.

        listing_exchange: String.
        Returns the primary listing exchange of the contract.

        conid: int.
        Returns the contract identifier of the order.

        conidEx: String.
        Returns the contract identifier of the order.

        clearing_id: String.
        Returns the clearing firm identifier.

        clearing_name: String.
        Returns the clearing firm identifier.

        liquidation_trade: String.
        Returns whether the order was part of an account liquidation or note.

        is_event_trading: String.
        Returns whether the order was part of event trading or not.
    """
    req_url = endpoints.base_url + endpoints.account_trades + "?" + (f"days={days}" if days is not None else "")
    req_tr = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_tr)
    return req_tr


def accountPositions(accountID: str, page: int = 0) -> requests.Response:
    """ 
    REQUEST OBJECT
        accountId: String. Required
        The account ID for which account should place the order.

        pageId: String. Required
        The “page” of positions that should be returned.
        One page contains a maximum of 100 positions.
        Pagination starts at 0.

        Query Params
        model: String.
        Code for the model portfolio to compare against.

        sort: String.
        Declare the table to be sorted by which column

        direction: String.
        The order to sort by.
        ‘a’ means ascending
        ‘d’ means descending

        period: String.
        period for pnl column
        Value Format: 1D, 7D, 1M

  
    RESPONSE OBJECT
        acctId: String.

        conid: int.
        Returns the contract ID of the position.

        contractDesc: String.
        Returns the local symbol of the order.

        position: float.
        Returns the total size of the position.

        mktPrice:  float.
        Returns the current market price of each share.

        mktValue:  float.
        Returns the total value of the order.

        avgCost: float.
        Returns the average cost of each share in the position times the multiplier.

        avgPrice: float.
        Returns the average cost of each share in the position when purchased.

        realizedPnl: float.
        Returns the total profit made today through trades.

        unrealizedPnl: float.
        Returns the total potential profit if you were to trade.

        exchs: null.
        Deprecated value.
        Always returns null.

        currency: String.
        Returns the traded currency for the contract.

        time: int.
        Returns amount of time in ms to generate the data.

        chineseName: String.
        Returns the Chinese characters for the symbol.

        allExchanges: String*.
        Returns a series of exchanges the given symbol can trade on.

        listingExchange: String.
        Returns the primary or listing exchange the contract is hosted on.

        countryCode: String.
        Returns the country code the contract is traded on.

        name: String.
        Returns the comapny name.

        assetClass: String.
        Returns the asset class or security type of the contract.

        expiry: String.
        Returns the expiry of the contract. Returns null for non-expiry instruments.

        lastTradingDay: String.
        Returns the last trading day of the contract.

        group: String.
        Returns the group or industry the contract is affilated with.

        putOrCall: String.
        Returns if the contract is a Put or Call option.

        sector: String.
        Returns the contract’s sector.

        sectorGroup: String.
        Returns the sector’s group.

        strike: int.
        Returns the strike of the contract.

        ticker: String.
        Returns the ticker symbol of the traded contract.

        undConid: int.
        Returns the contract’s underlyer.

        multiplier: float,
        Returns the contract multiplier.

        type: String.
        Returns stock type.

        hasOptions: bool.
        Returns if contract has tradable options contracts.

        fullName: String.
        Returns symbol name for requested contract.

        isUS: bool.
        Returns if the contract is US based or not.

        incrementRules: Array.
        Returns rules regarding incrementation for market data and order placemnet.

        lowerEdge: float,
        Returns lower edge value used to calculate increment.

        increment: float.
        Allowed incrementable value.

        displayRule: object.
        Returns an object containing display content for market data.

        magnification: int.
        Returns maginification or multiplier of contract

        displayRuleStep: Array.
        Contains various rules in the display object.

        decimalDigits: int.
        Returns average decimal digit for data display.

        lowerEdge: float.
        Returns lower edge value used to calculate increment.

        wholeDigits: int.
        Returns allowed display size.

        isEventContract: bool.
        Returns if the contract is an event contract or not.

        pageSize: int.
        Returns the content size of the request.
    """
    req_url = endpoints.base_url + endpoints.account_positions(accountID, page)
    req_pos = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_pos)
    return req_pos

def accountPosition(accountID: str, conid: str) -> requests.Response:
    req_url = endpoints.base_url + endpoints.account_position(accountID, conid)
    req_pos = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_pos)
    return req_pos


def createAlert(accountID: str, json_content: dict) -> requests.Response:

    """

    REQUEST OBJECT

    Path Params

    accountId: String.  Required
    Identifier for the unique account to retrieve information from.
    Value Format: “DU1234567”

    Body Prams

    alertName: String. Required
    Used as a human-readable identifier for your created alert.
    Format Structure: “Alert Name”

    alertMessage: String. Required
    The body content of what your alert will report once triggered
    Value Format: “MESSAGE TEXT”

    alertRepeatable: int. Required
    Boolean number (0, 1) signifies if an alert can be triggered more than once.
    A value of ‘1’ is required for MTA alerts
    Value Format:

    email: String. Required if ‘sendMessage’ == 1
    Email address you want to send email alerts to
    Value Format:

    expireTime: String. Required if ‘tif’ == ‘GTD’
    Used with a tif of “GTD” only. Signifies time when the alert should terminate if no alert is triggered.
    Value Format: “YYYYMMDD-HH:mm:ss”

    iTWSOrdersOnly: int. Optional
    Boolean number (0, 1) to allow alerts to trigger alerts through the mobile app.
    Value Format: 1

    outsideRth: int. Required
    Boolean number (0, 1) to allow the alert to trigger outside of regular trading hours.
    Value Format: 1

    sendMessage: int. Optional
    Boolean number (0, 1) to allow alerts to trigger email messages
    Value Format: 1

    showPopup: int. Optional
    Boolean number (0, 1) to allow alerts to trigger TWS Pop-up messages
    Value Format: 1

    tif: String.. Required
    Time in Force duration of alert. Allowed: [“GTC”, “GTD”]
    Value Format: “DAY”

    conditions: List of Arrays. Required
    Container for all conditions applied for an alert to trigger.
    Required field.
    Value Format:[ {…} ]

        conidex: String. Required
        Concatenation of conid and exchange. Formatted as “conid@exchange”
        Value Format: “265598@SMART”

        logicBind: String. Required
        Describes how multiple conditions should behave together.
        Allowed values are: {“a”: “AND”, “o”: “OR”, “n”: “END”}
        Value Format: “a”

        operator: String. Required
        Indicates whether the trigger should be above or below the given value.
        Value Format:”>=”

        timeZone: String. Required for MTA alerts
        Only needed for some MTA alert condition
        Value Format: “US/Eastern”

        triggerMethod: String. Required
        Pass the string representation of zero, “0”
        Value Format: “0”

        type: int. Required
        Designate what condition type to use.
        Allowed values: {1: Price, 3: Time, 4: Margin, 5: Trade, 6: Volume, 7: MTA market, 8: MTA Position, 9: MTA Account Daily PnL}
        Value Format: 1

        value: String. Required
        Trigger value based on Type. Allows a default value of “*”.
        Value Format: “195.00”, “YYYYMMDD-HH:mm:ss”

    
    Response Object:
    Returns a single json object

    request_id: integer. Always returns ‘null’

    order_id: integer. Signifies tracking ID for given alert.

    success: boolean. Displays result status of alert request

    text: String. Response message to clarify success status reason.

    order_status: String. Returns ‘null’

    warning_message: String. Returns ‘null’
    }

    """

    req_url = endpoints.base_url + endpoints.alert(accountID)
    req_al = requests.post(url=req_url, json=json_content, verify=False)
    checkResponse(req_url, req_al)
    return req_al

def getAlerts(accountID: str) -> requests.Response:
    req_url = endpoints.base_url + endpoints.alerts(accountID)
    req_al = requests.get(url=req_url, verify=False)
    checkResponse(req_url, req_al)
    return req_al
