
#ifndef __BACKTESTER_H
#define __BACKTESTER_H

#include <string>
#include "Bars.h"

class BackTester
{
    Bars* barsRef;
    float takeProfThresh;
    float stopLossThresh;

    const int maxTrades = 1;
    float pl = 0.0;
    float* plArray;
    int currTradeNo = 0;
    int openTrades = 0;

    const char* outputPath = "../output/stratReport.txt";
    const char* tradeLogPath = "../output/tradeLog.txt";
    string tradeLog = "";

    class Trade;
    Trade** execTrades;

    int closeTrade(Trade*, int, string);
    void logTrade(string, int, string);
   

    public:

    BackTester(Bars*, float = 0.0, float = 0.0);

    //*** INTERFACE ***//
    int openTrade(int, int, string, float = 1.0);
    void closeTrades(int, int, string, bool = false, bool = false);
    void updateTrades(int);

    void printResults();

};


#endif