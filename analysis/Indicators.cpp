

#include "Indicators.h"
#include "Bars.cpp"

#include <string>

using namespace std;


/******* INDICATOR STANDARD TEMPLATE ********/

template <class T> Indicator<T>::Indicator(Bars* dp, const string name)
{
    this->dp = dp;
    this->outputDirectory = dp->getoutputDir() + name + dp->getoutputExt();
    this->indicatorArray = new T* [dp->getnumBars()];
    // Initialize classes holding indicator data for each point:
    for (int i = 0; i < dp->getnumBars(); i++)
    {
        T* indicatorData = new T();
        indicatorArray[i] = indicatorData;
    }
}


template <class T> void Indicator<T>::printIndicator()
{
    //First ensure old file is clear:
    FILE* fp0 = fopen(outputDirectory.c_str(), "w");
    if (fp0 != NULL) fclose(fp0);

    //Open file for appending:
    FILE* fp = fopen(outputDirectory.c_str(), "a");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening outputDirectory for appending.\n");
        return;
    }
    for (int i = 0; i < this->dp->getnumBars(); i++)
    {
        fprintf(fp, "%s\n", this->indicatorArray[i]->toString().c_str());
    }
    fclose(fp);
}



/******* INDICATORS ********/

namespace Indicators
{
   
class RSI : public Indicator<IndicatorBars::RSI> 
{
    int timePeriod;

    public:

    RSI(Bars* dp, const string name = "RSI", const int dayTimePeriod = 14) : Indicator(dp, name)
    { this->timePeriod = dayTimePeriod * (24 / dp->gettimePeriod()) ; }
    
    void computeIndicator()
    {
        float totalUp = 0;
        float totalDown = 0;

        for (int d = 1; d < dp->getnumBars(); d++)
        {
            indicatorArray[d]->change = dp->getBar(d)->getclose() - dp->getBar(d-1)->getclose();

            // Computing RSI starting values:
            if (d == timePeriod)
            {
                for (int i = (d + 1) - timePeriod; i <= d; i++)
                {
                    if (indicatorArray[i]->change > 0)  { totalUp += indicatorArray[i]->change; }
                    else  { totalDown -= indicatorArray[i]->change; }
                }

                indicatorArray[d]->avgUp = (float)(totalUp / timePeriod);
                indicatorArray[d]->avgDown = (float)(totalDown / timePeriod);
                indicatorArray[d]->RSI = 100 - (float)(100 / (1 + (float)(indicatorArray[d]->avgUp / indicatorArray[d]->avgDown)));
            }

            // Computing RSI for successive values:
            else if (d > timePeriod)
            {
                float change = indicatorArray[d]->change;
                indicatorArray[d]->avgUp = (float)(((timePeriod - 1) * (indicatorArray[d-1]->avgUp) + (change > 0) * (change)) / timePeriod);
                indicatorArray[d]->avgDown = (float)(((timePeriod - 1) * (indicatorArray[d-1]->avgDown) - (change < 0) * (change)) / timePeriod);
                indicatorArray[d]->RSI = 100 - (float)(100 / (1 + (float)(indicatorArray[d]->avgUp / indicatorArray[d]->avgDown)));
            }
        }
    }
};


class LocalMin : public Indicator<IndicatorBars::LocalStat>
{
    protected:
    float m = 1.0;

    public:
    LocalMin(Bars* dp, const string name = "LocalMin") : Indicator(dp, name)
    {}

    void computeIndicator()
    {
        int d = 1;
        int leftDepth;
        int rightDepth;
        while (d < dp->getnumBars())
        {
            leftDepth = 0;
            while (d < dp->getnumBars() && 
                (this->m) * dp->getBar(d-1)->getclose() > (this->m) * dp->getBar(d)->getclose()) 
            { 
                leftDepth++;
                d++; 
            }
            if (leftDepth != 0)
            {
                this->indicatorArray[d-1]->leftDepth = leftDepth;
                this->indicatorArray[d-1]->m = (int) this->m;
                this->indicatorArray[d-1]->leftChange = 
                    ((float)(-1))*(this->m) * ((float)100) * (this->dp->getBar(d-1)->getclose() - this->dp->getBar(d-1-leftDepth)->getclose()) / this->dp->getBar(d-1)->getclose();
                rightDepth = 0;
                while (d < dp->getnumBars() && 
                    (this->m) * dp->getBar(d-1)->getclose() < (this->m) * dp->getBar(d)->getclose()) 
                { 
                    rightDepth++;
                    d++; 
                }
                this->indicatorArray[d-1-rightDepth]->rightDepth = rightDepth;
                this->indicatorArray[d-1-rightDepth]->rightChange = 
                    (this->m) * ((float)100) * (this->dp->getBar(d-1)->getclose() - this->dp->getBar(d-1-rightDepth)->getclose()) / this->dp->getBar(d-1)->getclose();
            }
            else { d++; }
        }
    }
};

class LocalMax : public LocalMin
{ 
    public:
    LocalMax(Bars* dp, const string name = "LocalMax") : LocalMin(dp, name)
    { this->m = -1.0; }
};


class Divergence : public Indicator<IndicatorBars::Divergence>
{
    /*** (Elementary) divergence parameters ***/
    int minDivPeriod = 2 * (24 / dp->gettimePeriod());
    int maxDivPeriod = 14 * (24 / dp->gettimePeriod());

    
    /*** Indicators required ***/
    RSI* RSIindic;
    LocalMax* LocalMaxIndic;
    LocalMin* LocalMinIndic;

    public:

    Divergence(Bars* dp, RSI* RSIindic, LocalMax* LocalMaxIndic, LocalMin* LocalMinIndic, const string name = "Divergence") 
        : Indicator(dp, name)
    {
        this->RSIindic = RSIindic;
        this->LocalMaxIndic = LocalMaxIndic;
        this->LocalMinIndic = LocalMinIndic;
    }

    void computeIndicator()
    {
        //First ensure other indicators are computed:
        RSIindic->computeIndicator();
        LocalMinIndic->computeIndicator();
        LocalMaxIndic->computeIndicator();
        // Then start iterating through each Bar:
        for (int d = 1; d < dp->getnumBars(); d++)
        {
            // Find a local maximum
            if (LocalMaxIndic->getIndicatorBar(d)->isPresent())
            {
                // Iterate back until previous local maximum
                for (int i = minDivPeriod; i < maxDivPeriod; i++)
                {
                    if ((d-i > 0) && LocalMaxIndic->getIndicatorBar(d-i)->isPresent())
                    {
                        // Test divergence conditions:
                        if (dp->getBar(d)->getclose() > dp->getBar(d-i)->getclose()
                         && RSIindic->getIndicatorBar(d)->RSI < RSIindic->getIndicatorBar(d-i)->RSI)
                        {
                            this->indicatorArray[d]->leftPos = d-i;
                            this->indicatorArray[d]->rightPos = d;
                            this->indicatorArray[d]->divPoints = this->indicatorArray[d-i]->divPoints + 1;
                            this->indicatorArray[d]->m = -1;
                        }
                        break;
                    }
                }
            }

            // Find a local minimum
            if (LocalMinIndic->getIndicatorBar(d)->isPresent())
            {
                // Iterate back until previous local minimum
                for (int i = minDivPeriod; i < maxDivPeriod; i++)
                {
                    if ((d-i > 0) && LocalMinIndic->getIndicatorBar(d-i)->isPresent())
                    {
                        // Test divergence conditions:
                        if (dp->getBar(d)->getclose() < dp->getBar(d-i)->getclose()
                         && RSIindic->getIndicatorBar(d)->RSI > RSIindic->getIndicatorBar(d-i)->RSI)
                        {
                            this->indicatorArray[d]->leftPos = d-i;
                            this->indicatorArray[d]->rightPos = d;
                            this->indicatorArray[d]->divPoints = this->indicatorArray[d-i]->divPoints + 1;
                            this->indicatorArray[d]->m = 1;
                        }
                        break;
                    }
                }
            }
        }
    }
};

}






namespace TestDivConditions
{
    class LocalStat
    {

        int LleftDepth = 0;
        float LleftChange = 0.0;
        int LrightDepth = 0;
        float LrightChange = 0.0;

        int RleftDepth = 0;
        float RleftChange = 0.0;
        int RrightDepth = 0;
        float RrightChange = 0.0;

        /*** Method to test adjacent stationary points on divergence ***/
        bool testAdjStat(IndicatorBars::LocalStat* LStat, IndicatorBars::LocalStat* RStat)
        {
            return (LStat->leftDepth > this->LleftDepth
            &&  LStat->leftChange > this->LleftChange 
            &&  LStat->rightDepth > this->LrightDepth
            &&  LStat->rightChange > this->LrightChange
            &&  RStat->leftDepth > this->RleftDepth
            &&  RStat->leftChange > this->RleftChange 
            &&  RStat->rightDepth > this->RrightDepth
            &&  RStat->rightChange > this->RrightChange
            );
        }

    };

};










/// TEST ///


Indicators::Divergence* initDivergences(Bars* Barsobj)
{
    Indicators::RSI* RSIObj = new Indicators::RSI(Barsobj, "RSI");
    Indicators::LocalMin* LocalMinObj = new Indicators::LocalMin(Barsobj, "LocalMin");
    Indicators::LocalMax* LocalMaxObj = new Indicators::LocalMax(Barsobj, "LocalMax");
    Indicators::Divergence* DivergenceObj = new Indicators::Divergence(Barsobj, RSIObj, LocalMaxObj, LocalMinObj, "Divergence");

    DivergenceObj->computeIndicator();

    Barsobj->printBars();
    RSIObj->printIndicator();
    DivergenceObj->printIndicator();

    return DivergenceObj;
}


/// TEST ///
