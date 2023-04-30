
#ifndef __INDICATOR_H
#define __INDICATOR_H

#include "Bars.h"

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

#endif //__INDICATOR_H