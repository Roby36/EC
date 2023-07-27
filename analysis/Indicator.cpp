
#include "Indicator.h"

/******* INDICATOR STANDARD TEMPLATE ********/

template <class T> Indicator<T>::Indicator(Bars* dp, const std::string name, const std::string logDirectory)
    : dp(dp)
{
    this->outputDirectory = dp->getoutputDir() + name + dp->getoutputExt();
    this->logDirectory    = logDirectory + name + dp->getoutputExt();
    for (int i = 0; i < MAXBARS; i++) {
        indicatorArray[i] = new T();
    }
}

template <class T> Indicator<T>::~Indicator() 
{
    for (int i = 0; i < MAXBARS; i++) {
        delete(this->indicatorArray[i]);
    }
}

template <class T> void Indicator<T>::printIndicator()
{
    //First ensure old file is clear:
    FILE* fp0 = fopen(outputDirectory.c_str(), "w");
    FILE* lp0 = fopen(logDirectory.c_str(), "w");
    if (fp0 != NULL) fclose(fp0);
    if (lp0 != NULL) fclose(lp0);
    //Open file for appending:
    FILE* fp = fopen(outputDirectory.c_str(), "a");
    FILE* lp = fopen(logDirectory.c_str(), "a");
    if (fp == NULL || lp == NULL) {
        fprintf(stderr, "Error opening outputDirectory or logDirectory for appending.\n");
        return;
    }
    for (int i = 0; i < this->dp->getnumBars(); i++) {
        fprintf(fp, "%s\n", this->indicatorArray[i]->toString().c_str());
        if (this->indicatorArray[i]->isPresent()) 
            fprintf(lp, "%s: %s\n", this->dp->getBar(i)->date_time_str,
            this->indicatorArray[i]->logString().c_str());    
    }
    fclose(fp);
    fclose(lp);
}
