
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
    T** indicatorArray;
     
    public:
    Indicator(Bars*, const string);
    
    T* getIndicatorBar(int i) { return this->indicatorArray[i]; }
    virtual void computeIndicator() = 0;
    void printIndicator(); 
    void Delete();
};


/******* INDICATOR BARS ********/

namespace IndicatorBars
{

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
        int leftPos = 0;
        int rightPos = 0;
        int m = 0;

        bool isPresent() { return (m != 0); }

        string toString() { return to_string(this->divPoints); }
    };

    class BollingerBands
    {
        public:

        float bollMiddle = 0, bollUpper = 0, bollLower = 0;

        bool crossUpperUp = false, crossUpperDown = false;
        bool crossMiddleUp = false, crossMiddleDown = false;
        bool crossLowerUp = false, crossLowerDown = false;
    };

    class JCandleSticks
    {
        public:

        bool bullEngulf = false, bearEngulf = false;
        bool bullHarami = false, bearHarami = false;
        bool piercing = false, darkCloud = false;
        bool morningStar = false, eveningStar = false;
        bool hammer = false;
        bool dfDoji = false, llDoji = false, gsDoji = false;
    };

    class MACD;

};

#endif