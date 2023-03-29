
#ifndef __INDICATORS_H
#define __INDICATORS_H

#include <string>

#include "Bars.h"

/******* INDICATOR STANDARD TEMPLATE ********/

template <class T> class Indicator  
{
    protected:
    Bars* dp; // access to Bars data
    string outputDirectory;
     

    public:
    Indicator(Bars*, const string);
    
    T** indicatorArray;
    virtual void computeIndicator() = 0;
    void printIndicator(); 
};


/******* INDICATOR BARS ********/

namespace IndicatorBars
{
    namespace JCandleStick
    {

    };

    class RSI
    {
        public:
        float avgUp = 0, avgDown = 0, change = 0, RSI = 0;

        string toString() { return to_string(RSI); }
    };

    class LocalStat
    {
        public:
        int leftDepth = 0, rightDepth = 0;
        float leftChange = 0.0, rightChange = 0.0;
        int m = 0;

        bool isPresent() { return (m != 0); }

        string toString() { return to_string(isPresent()); }
    };

    class Divergence
    {
        public:

        int divPoints = 1;
        LocalStat* leftStat = NULL;
        LocalStat* rightStat = NULL;

        string toString() { return to_string(this->divPoints); }
    };

    class BollingerBands;
    class MACD;

};

#endif