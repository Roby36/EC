
#pragma once
#include "Bars.h"

/******* INDICATOR STANDARD TEMPLATE ********/
template <class T> class Indicator  
{
    protected:
    Bars* const dp; 
    std::string outputDirectory;
    std::string logDirectory;
    T* indicatorArray [MAXBARS];
    
    public:
    Indicator(Bars* dp, 
              const std::string name = "Unnamed", 
              const std::string logDirectory =  "../indicators_log/");
    ~Indicator();

    T* getIndicatorBar(int i) { return this->indicatorArray[i]; }
    virtual void computeIndicator() = 0;
    void printIndicator(); 
};
