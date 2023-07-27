
#pragma once

#include "Instrument.h"
#include "Execution.h"
#include "Order.h"

// Trades need cleaning up when array full to make up space!
#define MAXTRADES 1024

typedef struct MTrade 
{
    bool isOpen = true;

    /*const*/ int tradeId;
    /*const*/ std::string strategy;

    // Don't keep pointers in a serialized class!
    /*const*/ int instr_id;  // Use instrument id to define standard constructor
    /*const*/ Order      openingOrder;
    /*const*/ Order      closingOrder;
    Execution  openingExecution = Execution(); // initialize empty execution values
    Execution  closingExecution = Execution();

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
