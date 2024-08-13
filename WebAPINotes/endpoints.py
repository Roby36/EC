

tickle: str                 = "/tickle"
auth_status: str            = "iserver/auth/status"
base_url: str               = "https://localhost:5000/v1/api/"
secdef_search: str          = "/iserver/secdef/search"
secdef_info: str            = "/iserver/secdef/info"
secdef_strikes: str         = "/iserver/secdef/strikes"
marketdata_snapshot: str    = "iserver/marketdata/snapshot"
hmds_history: str           = "/hmds/history"
mkt_data_history: str       = "/iserver/marketdata/history"
accounts: str               = "/iserver/accounts"
questions_suppress: str     = "/iserver/questions/suppress"
account_trades: str         = "/iserver/account/trades"

alert = lambda accountID: f"/iserver/account/{accountID}/alert"
alerts = lambda accountID: f"/iserver/account/{accountID}/alerts"
accountID_orders = lambda accountID: f"/iserver/account/{accountID}/orders"
accountID_orderID = lambda accountID, orderID: f"/iserver/account/{accountID}/order/{orderID}"
order_status = lambda orderID: f"/iserver/account/order/status/{orderID}"
reply = lambda replyID: f"/iserver/reply/{replyID}"
account_positions = lambda accountID, page: f"portfolio/{accountID}/positions/{page}"
account_position = lambda accountID, conid: f"portfolio/{accountID}/position/{conid}"

