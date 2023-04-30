
#include "Trade.h"

Trade::Trade(Bars* barsRef,  int currTradeNo, int direction, int entryPos, int units)
{
    this->barsRef = barsRef; 
    this->direction = direction;
    this->entryPos = entryPos;
    this->exitPos = entryPos;
    this->units = units;
    this->tradeNo = currTradeNo;
}

bool Trade::close(int exitPos) 
{
    if (this->isActive()) {
        this->exitPos = exitPos;
        return true;
    }
    return false;
}

float Trade::currBal(int currPos) 
{
    if (this->isActive()) {
        return
        (this->units)*(this->multFactor)*(this->direction)*
        (this->barsRef->getBar(currPos)->close() - this->barsRef->getBar(this->entryPos)->close());
    } else {
        return
        (this->units)*(this->multFactor)*(this->direction)*
        (this->barsRef->getBar(this->exitPos)->close() - this->barsRef->getBar(this->entryPos)->close());
    }
}

bool Trade::isActive() {
    return this->exitPos == this->entryPos;
}

string Trade::print() 
{
    string dir;
    if (this->direction == 1) { dir = "long"; }
    else { dir = "short"; }

    return(
        " Trade no: " + to_string(this->tradeNo) + ";"
        + " Direction: " + dir + ";"
        + " Entry date: " + string(this->barsRef->getBar(entryPos)->date_time_str) + ";"
        + " Entry closing price: " + to_string(this->barsRef->getBar(entryPos)->close()) + ";"
        + " Exit date: " + string(this->barsRef->getBar(exitPos)->date_time_str) + ";"
        + " Exit closing price: " + to_string(this->barsRef->getBar(exitPos)->close()) + ";"
        + " Loss/profit: " + to_string(this->currBal(exitPos)) + ";"
        + "\n"
    );
}
    
