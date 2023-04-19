
#ifndef __INDICATORBAR_H
#define __INDICATORBAR_H

#include <string>

using namespace std;

class IndicatorBar
{
    public:

    virtual bool isPresent() = 0;
    virtual string toString() = 0;
    virtual string logString() = 0;
};

#endif // __INDICATORBAR_H