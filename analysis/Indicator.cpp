
#include "Indicator.h"

/******* INDICATOR STANDARD TEMPLATE ********/

template <class T> Indicator<T>::Indicator(Bars* dp, const string name, const string logDirectory)
{
    this->dp = dp;
    this->outputDirectory = dp->getoutputDir() + name + dp->getoutputExt();
    this->logDirectory = logDirectory + name + dp->getoutputExt();
    this->indicatorArray = new T* [dp->getnumBars()];
    // Initialize classes holding indicator data for each point:
    for (int i = 0; i < dp->getnumBars(); i++) {
        indicatorArray[i] = new T();
    }
}

template <class T> Indicator<T>::~Indicator() 
{
    for (int i = 0; i < dp->getnumBars(); i++) {
        delete(this->indicatorArray[i]);
    }
    delete(this->indicatorArray);
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
        if (this->indicatorArray[i]->isPresent()) {
            fprintf(lp, "%s: %s\n", this->dp->getBar(i)->date_time_str,
                this->indicatorArray[i]->logString().c_str());
        }
    }
    fclose(fp);
    fclose(lp);
}