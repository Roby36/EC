
#pragma once
#include "Bars.h"

class IndicatorBar
{
    public:

    bool is_computed = false;
    virtual bool isPresent()   = 0;
    virtual std::string toString()  = 0;
    virtual std::string logString() = 0;

};
