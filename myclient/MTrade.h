
#pragma once

#include "Instrument.h"
#include "Execution.h"
#include "Order.h"
#include "CommonMacros.h"

typedef struct MTrade 
{
    bool isOpen = false;

    /*const*/ int tradeId;
    /*const*/ std::string strategy;

    /*const*/ int instr_id;  // Use instrument id to define standard constructor
    /*const*/ Order      openingOrder;
    /*const*/ Order      closingOrder;
    Execution openingExecution = Execution(); // initialize empty execution values
    Execution closingExecution = Execution();

    MTrade()  // default constructor for unserialization (other parameters consequently set)
    :  tradeId(-1),
        strategy(std::string("")),
        instr_id(-1),
        openingOrder(Order()),
        closingOrder(Order())
    {
    }

    MTrade(const int tradeId,
           const int instr_id,
           const std::string strategy,
           const Order openingOrder,
           const Order closingOrder)
        : tradeId(tradeId), 
          strategy(strategy),
          instr_id(instr_id),
          openingOrder(openingOrder),
          closingOrder(closingOrder)
    {
    }

} MTrade_t;

class TradeData
{
    public:

    int numTrades = 0;
    MTrade_t* tradeArr[MAXTRADES];

};