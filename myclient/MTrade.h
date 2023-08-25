
#pragma once

#include "Instrument.h"
#include "Execution.h"
#include "Order.h"
#include "CommonMacros.h"
#include "CommonEnums.h"

struct MTrade 
{
    int tradeId = -1;
    TradeDirection dir = TradeDirection::LONG;
    std::string strategy_code = "";
    std::string opening_reason = "";
    std::string closing_reason = "";

    /** Backtesting parameters **/
    int bt_entry_bar = 0;
    int bt_exit_bar  = 0;
    double bt_order_quant = 0.0;

    /** Live trading parameters **/
    Order *     openingOrder       = NULL;
    Order *     closingOrder       = NULL;
    bool waiting_opening_execution = false;
    bool waiting_closing_execution = false;
    Execution * openingExecution   = NULL; // initialize empty execution values
    Execution * closingExecution   = NULL;

    ~MTrade();
};

class TradeData
{
    public:

    int numTrades = 0;
    MTrade * tradeArr[MAXTRADES];

};


