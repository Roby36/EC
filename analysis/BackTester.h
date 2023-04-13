
#ifndef __BACKTESTER_H
#define __BACKTESTER_H

#include <string>
#include "Bars.h"

class BackTester
{
    Bars* barsRef;
    float takeProfThresh;
    float stopLossThresh;
    int maxTrades;
    char* reportPath;
    char* logPath;

    float pl = 0.0f;
    float* plArray;
    int currTradeNo = 0;
    int openTrades = 0;

    
    string tradeLog = "";

    class Trade;
    Trade** execTrades;

    void logTrade(string, int, string);
   
    public:

    BackTester(Bars*, float = 0.0, float = 0.0, int = 3, char* = NULL, char* = NULL);

    //*** INTERFACE ***//
    int openTrade(int, int, string, float = 1.0);
    bool closeTrade(int, int, string);
    void closeTrades(int, int, string, bool = false, bool = false);
    void updateTrades(int);

    void printResults();
    void Delete();

};


#endif