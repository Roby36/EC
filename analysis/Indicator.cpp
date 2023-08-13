
#include "Indicator.h"

/******* INDICATOR STANDARD TEMPLATE ********/

template <class T> Indicator<T>::Indicator(Bars* dp, const int starting_bar, const std::string name, const std::string logDirectory)
    : dp(dp), starting_bar(starting_bar), name(name), logDirectory(logDirectory)
{
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

template <class T> void Indicator<T>::computeIndicator()
{
    for (int d = starting_bar; d < dp->getnumBars(); d++) {
        if ((this->getIndicatorBar(d))->is_computed)
            continue;
        this->computeIndicatorBar(d); // compute idnicator bar only if not computed yet
        this->getIndicatorBar(d)->is_computed = true; // mark indicator bar a s copmputed
    }
}

template <class T> void Indicator<T>::printIndicator(const std::string outputDir, const std::string str, const std::string outputExt)
{
    char * path    = (char *) malloc (OUTDIRCHAR);
    char * logpath = (char *) malloc (OUTDIRCHAR);
    strncpy(path, (outputDir + str + this->name + outputExt).c_str(), OUTDIRCHAR);
    strncpy(logpath, (this->logDirectory + str + this->name + outputExt).c_str(), OUTDIRCHAR);
    Bars::clear_file(path);
    Bars::clear_file(logpath);
    FILE* fp = fopen(path, "a");
    FILE* lp = fopen(logpath, "a");
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
    free(path);
    free(logpath);
}
