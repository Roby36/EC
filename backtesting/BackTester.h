
#pragma once
#include "Trade.h"
#include "../myclient/memdbg.h"

// YOU MUST ALLOCATE THE ARRAY STATICALLY
// ELSE COMPILER WON'T KNOW SIZE OF OBJECT
// HENCE HOW MUCH MEMORY TO ALLOCATE ON THE HEAP
// --> BUS ERROR!
#define MAXBTTRADES 1024

class BackTester
{
    Bars* const barsRef;
    const char* reportPath;
    const char* logPath;

    double pl = 0.0;
    double plArray [MAXBARS];
    int currTradeNo = 0;
    int openTrades = 0;

    std::string tradeLog = "";

    Trade * execTrades [MAXBTTRADES];

    void logTrade(std::string, int, std::string);
   
    public:

    BackTester(Bars* barsRef, 
               const char* reportPath, 
               const char* logPath);
    ~BackTester();

    //*** INTERFACE ***//
    int  openTrade(int, int, std::string, double = 1.0);
    bool closeTrade(int, int, std::string);
    void closeTrades(int, int, std::string, bool = false, bool = false, int = 0);
    void updateTrades(int, double, double);
    void printResults();
};
