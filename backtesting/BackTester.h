
#ifndef __BACKTESTER_H
#define __BACKTESTER_H

#include "Trade.cpp"

class BackTester
{
    Bars* barsRef;
    int maxTrades;
    const char* reportPath;
    const char* logPath;

    float pl = 0.0f;
    float* plArray;
    int currTradeNo = 0;
    int openTrades = 0;

    string tradeLog = "";

    Trade** execTrades;

    void logTrade(string, int, string);
   
    public:

    BackTester(Bars*, const int, const char*, const char*);

    //*** INTERFACE ***//
    int openTrade(int, int, string, float = 1.0);
    bool closeTrade(int, int, string);
    void closeTrades(int, int, string, bool = false, bool = false, int = 0);
    void updateTrades(int, float, float);
    void printResults();
    void Delete();

};


#endif