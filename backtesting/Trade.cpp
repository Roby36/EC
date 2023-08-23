
#include "Trade.h"

Trade::Trade(Bars* barsRef, int currTradeNo, int direction, int entryPos, int units)
    : barsRef(barsRef)
{
    this->direction = direction;
    this->entryPos  = entryPos;
    this->exitPos   = entryPos;
    this->units     = units;
    this->tradeNo   = currTradeNo;
}

bool Trade::close(int exitPos) 
{
    if (this->isActive()) {
        this->exitPos = exitPos;
        return true;
    }
    return false;
}

double Trade::currBal(int currPos) 
{
    int closing_pos = this->isActive() ? currPos : this->exitPos;
    return (this->units)*(this->multFactor)*(this->direction)*
        (this->barsRef->getBar(closing_pos)->close() - this->barsRef->getBar(this->entryPos)->close());
}

bool Trade::isActive() {
    return this->exitPos == this->entryPos;
}

std::string Trade::print() 
{
    std::string dir = this->direction == 1 ? "long" : "short";
    return(
          " Trade no: "            + std::to_string(this->tradeNo) + ";"
        + " Direction: "           + dir + ";"
        + " Entry date: "          + std::string   (this->barsRef->getBar(entryPos)->date_time_str) + ";"
        + " Entry closing price: " + std::to_string(this->barsRef->getBar(entryPos)->close())       + ";"
        + " Exit date: "           + std::string   (this->barsRef->getBar(exitPos)->date_time_str)  + ";"
        + " Exit closing price: "  + std::to_string(this->barsRef->getBar(exitPos)->close())        + ";"
        + " Loss/profit: "         + std::to_string(this->currBal(exitPos))                         + ";" 
    );
}
    
