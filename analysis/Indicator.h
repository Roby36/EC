
#pragma once
#include "Bars.h"

/******* INDICATOR STANDARD TEMPLATE ********/
template <class T> class Indicator  
{
    protected:
    Bars* const dp; 
    const int starting_bar;
    std::string outputDirectory;
    std::string logDirectory;
    T* indicatorArray [MAXBARS];
    
    public:
    Indicator(Bars* dp, 
              const int starting_bar,
              const std::string name = "Unnamed", 
              const std::string logDirectory =  "../indicators_log/");
    ~Indicator();

    T* getIndicatorBar(int i) { return this->indicatorArray[i]; }
    virtual void computeIndicatorBar(int& d) = 0;
    virtual void computeIndicator();
    void printIndicator(); 
};
