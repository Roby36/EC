
#ifndef __TRADE_H
#define __TRADE_H

#include "../analysis/Bars.h"

class Trade
{
    Bars* barsRef; 

    const float multFactor = 1.0;
    int tradeNo;
    int entryPos;
    int exitPos;
    int direction;

    float units = 1.0;

    public:

    Trade(Bars* barsRef,  int currTradeNo, int direction, int entryPos, int units);
    bool close(int exitPos);
    float currBal(int currPos);
    bool isActive();
    string print();

    /*** GETTERS ***/
    int getdir() const { return this->direction; }
    int getTradeNo() const { return this->tradeNo; }
    int getEntryPos() const { return this->entryPos; }    

};

#endif // __TRADE_H