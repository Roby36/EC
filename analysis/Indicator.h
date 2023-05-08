
#pragma once
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
    ~Indicator();

    T* getIndicatorBar(int i) { return this->indicatorArray[i]; }
    virtual void computeIndicator() = 0;
    void printIndicator(); 
};
