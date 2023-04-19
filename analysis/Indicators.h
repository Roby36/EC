
#ifndef __INDICATORS_H
#define __INDICATORS_H

#include <string>
#include "Bars.h"


class Indicators
{
    /******* INDICATOR STANDARD TEMPLATE ********/
    template <class T> class Indicator  
    {
        protected:
        Bars* dp; // access to Bars data
        string outputDirectory;
        string logDirectory;
        T** indicatorArray;
     
        public:
        Indicator(Bars*, const string, const string = "../indicatorslog/");
    
        T* getIndicatorBar(int i) { return this->indicatorArray[i]; }
        virtual void computeIndicator() = 0;
        void printIndicator(); 
        void Delete();
    };

    /******* INDICATOR CLASSES ********/
    class RSI;
    class LocalMin;
    class LocalMax;
    class Divergence;
    class BollingerBands;
    class JCandleSticks;

    public:

    /******* INDICATOR INSTANCES, CONSTRUCTOR, DELETE ********/

    Indicators(Bars*);
    void Delete();
    void printIndicators();

    RSI* RSI;
    LocalMin* LocalMin;
    LocalMax* LocalMax;
    Divergence* Divergence;
    BollingerBands* BollingerBands;
    JCandleSticks* JCandleSticks;

};

#endif