
#pragma once
#include "Order.h"

class MOrders 
{
    public:

    /// A Market order is an order to buy or sell at the market bid or offer price. A market order may increase the likelihood of a fill 
    /// and the speed of execution, but unlike the Limit order a Market order provides no price protection and may fill at a price far 
    /// lower/higher than the current displayed bid/ask.
    /// Products: BOND, CFD, EFP, CASH, FUND, FUT, FOP, OPT, STK, WAR
    static Order MarketOrder(std::string action, Decimal quantity, std::string orderRef = "");

    /// A Market-on-Close (MOC) order is a market order that is submitted to execute as close to the closing price as possible.
    /// Products: CFD, FUT, STK, WAR
    static Order MarketOnClose(std::string action, Decimal quantity, std::string orderRef = "");

    /// A Market-on-Open (MOO) order combines a market order with the OPG time in force to create an order that is automatically
    /// submitted at the market's open and fills at the market price.
    /// Products: CFD, STK, OPT, WAR
    static Order MarketOnOpen(std::string action, Decimal quantity, std::string orderRef = "");

    /// A Limit order is an order to buy or sell at a specified price or better. The Limit order ensures that if the order fills, 
    /// it will not fill at a price less favorable than your limit price, but it does not guarantee a fill.
    /// Products: BOND, CFD, CASH, FUT, FOP, OPT, STK, WAR
    static Order LimitOrder(std::string action, Decimal quantity, double limitPrice, std::string orderRef = "");

    /// A Market-to-Limit (MTL) order is submitted as a market order to execute at the current best market price. If the order is only 
    /// partially filled, the remainder of the order is canceled and re-submitted as a limit order with the limit price equal to the price 
    /// at which the filled portion of the order executed.
    /// Products:CFD, FUT, FOP, OPT, STK, WAR
    static Order MarketToLimit(std::string action, Decimal quantity, std::string orderRef = "");

    /// A Market with Protection order is a market order that will be cancelled and 
    /// resubmitted as a limit order if the entire order does not immediately execute at the market price. The limit price set to be 
    /// close to the current market price, slightly higher for a sell order and lower for a buy order.
    /// Products: FUT, FOP
    static Order MarketWithProtection(std::string action, Decimal quantity, std::string orderRef = "");

    /// A Stop order is an instruction to submit a buy or sell market order if and when the user-specified stop trigger price is attained or 
    /// penetrated. A Stop order is not guaranteed a specific execution price and may execute significantly away from its stop price. A Sell 
    /// Stop order is always placed below the current market price and is typically used to limit a loss or protect a profit on a long stock 
    /// position. A Buy Stop order is always placed above the current market price. It is typically used to limit a loss or help protect a 
    /// profit on a short sale.
    /// Products: CFD, BAG, CASH, FUT, FOP, OPT, STK, WAR
    static Order Stop(std::string action, Decimal quantity, double stopPrice, std::string orderRef = "");

    /// A Stop-Limit order is an instruction to submit a buy or sell limit order when the user-specified stop trigger price is attained or 
    /// penetrated. The order has two basic components: the stop price and the limit price. When a trade has occurred at or through the stop 
    /// price, the order becomes executable and enters the market as a limit order, which is an order to buy or sell at a specified price or better.
    /// Products: CFD, CASH, FUT, FOP, OPT, STK, WAR
    static Order StopLimit(std::string action, Decimal quantity, double limitPrice, double stopPrice, std::string orderRef = "");

    /// A Stop with Protection order combines the functionality of a stop limit order with a market with protection order. The order is set 
    /// to trigger at a specified stop price. When the stop price is penetrated, the order is triggered as a market with protection order, 
    /// which means that it will fill within a specified protected price range equal to the trigger price +/- the exchange-defined protection 
    /// point range. Any portion of the order that does not fill within this protected range is submitted as a limit order at the exchange-defined 
    /// trigger price +/- the protection points.
    /// Products: FUT
    static Order StopWithProtection(std::string action, Decimal quantity, double stopPrice, std::string orderRef = "");

    /// Create combination orders that include options, stock and futures legs (stock legs can be included if the order is routed 
    /// through SmartRouting). Although a combination/spread order is constructed of separate legs, it is executed as a single transaction 
    /// if it is routed directly to an exchange. For combination orders that are SmartRouted, each leg may be executed separately to ensure 
    /// best execution.
    /// Products: OPT, STK, FUT
    static Order ComboMarketOrder(std::string action, Decimal quantity, bool nonGuaranteed, std::string orderRef = "");

    /// One-Cancels All (OCA) order type allows an investor to place multiple and possibly unrelated orders assigned to a group. The aim is 
    /// to complete just one of the orders, which in turn will cause TWS to cancel the remaining orders. The investor may submit several 
    /// orders aimed at taking advantage of the most desirable price within the group. Completion of one piece of the group order causes 
    /// cancellation of the remaining group orders while partial completion causes the group to rebalance. An investor might desire to sell 
    /// 1000 shares of only ONE of three positions held above prevailing market prices. The OCA order group allows the investor to enter prices 
    /// at specified target levels and if one is completed, the other two will automatically cancel. Alternatively, an investor may wish to take 
    /// a LONG position in eMini S&P stock index futures in a falling market or else SELL US treasury futures at a more favorable price. 
    /// Grouping the two orders using an OCA order type offers the investor two chance to enter a similar position, while only running the risk 
    /// of taking on a single position.
    /// Products: BOND, CASH, FUT, FOP, STK, OPT, WAR
    static void OneCancelsAll(std::string ocaGroup, Order& ocaOrder, int ocaType, std::string orderRef = "");


};

