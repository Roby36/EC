
#pragma once
#include "Bars.h"

class IndicatorBar
{
    public:

    virtual bool isPresent()   = 0;
    virtual string toString()  = 0;
    virtual string logString() = 0;
};
