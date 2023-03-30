
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

    class Trade;
    Trade** execTrades;

    public:

    BackTester(Bars*, float = 0.0, float = 0.0);

    //*** INTERFACE ***//
    bool openTrade(int, int, float = 1.0);
    void closeTrade(Trade*, int);
    void updateTrades(int);

    void printResults();
};


#endif