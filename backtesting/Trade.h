
#pragma once
#include "../analysis/Bars.h"

class Trade
{
    Bars* const barsRef; 

    const double multFactor = 1.0;
    int tradeNo;
    int entryPos;
    int exitPos;
    int direction;

    double units = 1.0;

    public:

    Trade(Bars* barsRef,  int currTradeNo, int direction, int entryPos, int units);
    bool close(int exitPos);
    double currBal(int currPos);
    bool isActive();
    std::string print();

    /*** GETTERS ***/
    int getdir()      const { return this->direction;}
    int getTradeNo()  const { return this->tradeNo;}
    int getEntryPos() const { return this->entryPos;}    

};
