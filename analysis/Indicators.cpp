
#include "Indicators.h"
#include "Bars.cpp"

#include <string>
#include <math.h>

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
        indicatorArray[i] = new T();
    }
}


template <class T> void Indicator<T>::Delete()
{
    for (int i = 0; i < dp->getnumBars(); i++)
    {
        delete(this->indicatorArray[i]);
    }
    delete(this->indicatorArray);
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

    RSI(Bars* dp, const string name = "RSI", const int TimePeriod = 14) : Indicator(dp, name)
    { this->timePeriod = TimePeriod; }
    
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
    int m = 1;

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
                    (float)((-1)*(this->m)) * ((float)100) * (this->dp->getBar(d-1)->getclose() - this->dp->getBar(d-1-leftDepth)->getclose()) / this->dp->getBar(d-1)->getclose();
                rightDepth = 0;
                while (d < dp->getnumBars() && 
                    (this->m) * dp->getBar(d-1)->getclose() < (this->m) * dp->getBar(d)->getclose()) 
                { 
                    rightDepth++;
                    d++; 
                }
                this->indicatorArray[d-1-rightDepth]->rightDepth = rightDepth;
                this->indicatorArray[d-1-rightDepth]->rightChange = 
                    ((float)(this->m)) * ((float)100) * (this->dp->getBar(d-1)->getclose() - this->dp->getBar(d-1-rightDepth)->getclose()) / this->dp->getBar(d-1)->getclose();
            }
            else { d++; }
        }
    }
};

class LocalMax : public LocalMin
{ 
    public:
    LocalMax(Bars* dp, const string name = "LocalMax") : LocalMin(dp, name)
    { this->m = -1; }
};


class Divergence : public Indicator<IndicatorBars::Divergence>
{
    /*** (Elementary) divergence parameters ***/
    int minDivPeriod;
    int maxDivPeriod;

    /*** Indicators required ***/
    RSI* RSIindic;
    LocalMax* LocalMaxIndic;
    LocalMin* LocalMinIndic;

    public:

    Divergence(Bars* dp, 
        RSI* RSIindic, LocalMax* LocalMaxIndic, LocalMin* LocalMinIndic, int minDivPeriod = 2, int maxDivPeriod = 28,
        const string name = "Divergence") 
        : Indicator(dp, name)
    {
        this->RSIindic = RSIindic;
        this->LocalMaxIndic = LocalMaxIndic;
        this->LocalMinIndic = LocalMinIndic;
        this->minDivPeriod = minDivPeriod;
        this->maxDivPeriod = maxDivPeriod;
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


class BollingerBands : public Indicator<IndicatorBars::BollingerBands>
{
    /*** Bollinger Bands parameters ***/
    float stDevUp, stDevDown;
    int timePeriod;

    public:

    BollingerBands(Bars* dp, 
        float stDevUp = 2.0f, float stDevDown = 2.0f, int timePeriod = 20,
        const string name = "BB") 
        : Indicator(dp, name)
    {
        this->stDevUp = stDevUp;
        this->stDevDown = stDevDown;
        this->timePeriod = timePeriod;
    }

    void computeIndicator()
    {
        for (int i = this->timePeriod; i < dp->getnumBars(); i++)
        {
            // Compute average, or bollMiddle value:
            float midSum = 0;
            for (int j = 0; j < this->timePeriod; j++)
            {
                midSum += dp->getBar(i-j)->getclose();
            }
            this->indicatorArray[i]->bollMiddle = (float) (midSum / (float) this->timePeriod);

            // Compute standard deviation:
            float SdSum = 0;
            for (int j = 0; j < this->timePeriod; j++)
            {
                SdSum += powf((dp->getBar(i-j)->getclose() - this->indicatorArray[i]->bollMiddle), (float) 2);
            }
            float SD = sqrtf((float) (SdSum / (float) this->timePeriod));

            // Compute upper & lower bands:
            this->indicatorArray[i]->bollLower = this->indicatorArray[i]->bollMiddle - SD * this->stDevDown;
            this->indicatorArray[i]->bollUpper = this->indicatorArray[i]->bollMiddle + SD * this->stDevUp;


            //** CROSSOVERS **//

            if (i > this->timePeriod)
            {
                if (dp->getBar(i)->getclose() > this->indicatorArray[i]->bollUpper
                    && dp->getBar(i-1)->getclose() < this->indicatorArray[i-1]->bollUpper)
                {
                    this->indicatorArray[i]->crossUpperUp = true;
                }
                if (dp->getBar(i)->getclose() < this->indicatorArray[i]->bollUpper
                    && dp->getBar(i-1)->getclose() > this->indicatorArray[i-1]->bollUpper)
                {
                    this->indicatorArray[i]->crossUpperDown = true;
                }

                if (dp->getBar(i)->getclose() > this->indicatorArray[i]->bollMiddle
                    && dp->getBar(i-1)->getclose() < this->indicatorArray[i-1]->bollMiddle)
                {
                    this->indicatorArray[i]->crossMiddleUp = true;
                }
                if (dp->getBar(i)->getclose() < this->indicatorArray[i]->bollMiddle
                    && dp->getBar(i-1)->getclose() > this->indicatorArray[i-1]->bollMiddle)
                {
                    this->indicatorArray[i]->crossMiddleDown = true;
                }

                if (dp->getBar(i)->getclose() > this->indicatorArray[i]->bollLower
                    && dp->getBar(i-1)->getclose() < this->indicatorArray[i-1]->bollLower)
                {
                    this->indicatorArray[i]->crossLowerUp = true;
                }
                if (dp->getBar(i)->getclose() < this->indicatorArray[i]->bollLower
                    && dp->getBar(i-1)->getclose() > this->indicatorArray[i-1]->bollLower)
                {
                    this->indicatorArray[i]->crossLowerDown = true;
                }
            }
        }
    }
};


class JCandleSticks : public Indicator<IndicatorBars::JCandleSticks>
{
    /*** JCandleSticks parameters ***/
    float hammerSize;
    float dojiSize;
    int dojiExtremes;

    public:

    JCandleSticks(Bars* dp, 
        float hammerSize = 3.0f, float dojiSize = 8.0f, int dojiExtremes = 3,
        const string name = "JCandleSticks") 
        : Indicator(dp, name)
    {
        this->hammerSize = hammerSize;
        this->dojiSize = dojiSize;
        this->dojiExtremes = dojiExtremes;
    }

    void computeIndicator()
    {
        for (int d = 2; d < dp->getnumBars(); d++)
        {
            float currOpen = this->dp->getBar(d)->getopen();
            float currClose = this->dp->getBar(d)->getclose();
            float currLow = this->dp->getBar(d)->getlow();
            float currHigh = this->dp->getBar(d)->gethigh();

            float prevOpen = this->dp->getBar(d-1)->getopen();
            float prevClose = this->dp->getBar(d-1)->getclose();
            float prevLow = this->dp->getBar(d-1)->getlow();
            float prevHigh = this->dp->getBar(d-1)->gethigh();

            float prevprevOpen = this->dp->getBar(d-2)->getopen();
            float prevprevClose = this->dp->getBar(d-2)->getclose();


        //** ENGULFMENTS, HARAMI, PIERCING, DARK CLOUD **//

            if (prevOpen > prevClose     // Bearish candlestick
                && currOpen < currClose) // followed by bullish candlestick
            {
                if (currOpen < prevClose 
                    && currClose > prevOpen) 
                {    
                    this->indicatorArray[d]->bullEngulf = true;
                }

                if (currClose < prevOpen
                    && currOpen > prevClose)
                {
                    this->indicatorArray[d]->bullHarami = true;
                }

                if (currOpen < prevLow
                    && currClose > (float)((prevOpen + prevClose)/ (float) 2) )
                {
                    this->indicatorArray[d]->piercing = true;
                }
            }

            if (prevOpen < prevClose     // Bullish candlestick
                && currOpen > currClose) // followed by bearish candlestick
            {
                if (currOpen > prevClose 
                    && currClose < prevOpen)  
                { 
                    this->indicatorArray[d]->bearEngulf = true;
                }

                if (currOpen < prevClose
                 && currClose > prevOpen)
                {
                    this->indicatorArray[d]->bearHarami = true;
                }

                // Dark cloud criteria:
                if (currOpen > prevHigh
                    && currClose < (float)((prevOpen + prevClose)/ (float) 2))
                {
                    this->indicatorArray[d]->darkCloud = true;
                }
            }

        //** MORNING & EVENING STARS **//
            
            if (prevprevOpen > prevprevClose                        // first bearish candlestick
                && currOpen < currClose                             // third bullish candlestick
                && prevprevClose > prevOpen                         // second day opens and closes below close of first day
                && prevprevClose > prevClose                        // second day opens and closes below open of third day 
                && currOpen > prevOpen 
                && currOpen > prevClose)
            {
                this->indicatorArray[d]->morningStar = true;
            }
            
            if (prevprevOpen < prevprevClose                        // first bullish candlestick
                && currOpen > currClose                             // third bearish candlestick
                && prevprevClose < prevOpen                         // second day opens and closes above close of first day
                && prevprevClose < prevClose                        // second day opens and closes above open of third day
                && currOpen < prevOpen 
                && currOpen < prevClose)
            {
                this->indicatorArray[d]->eveningStar = true;
            }

        //** HAMMERS **//

            if ((max(currOpen,currClose) - currLow) > this->hammerSize * (max(currOpen,currClose) - min(currOpen,currClose)) 
                && (max(currOpen,currClose) - min(currOpen,currClose)) > (currHigh - max(currOpen,currClose)))
            {
                this->indicatorArray[d]->hammer = true;
            }

        //** DOJIS **//

            if ((currHigh - currLow) > this->dojiSize * (max(currOpen,currClose) - min(currOpen,currClose)))
            { 
                if (min(currOpen,currClose) > currLow + (currHigh - currLow) * ((1 - (float)1 / (float) this->dojiExtremes)))
                {
                    this->indicatorArray[d]->dfDoji = true;
                }

                else if (max(currOpen,currClose) < currLow + (currHigh - currLow) * ((float)1 / (float) this->dojiExtremes))
                {
                    this->indicatorArray[d]->gsDoji = true;
                }

                else
                {
                    this->indicatorArray[d]->llDoji = true;
                }
            }
        }
    }
};



}








/// TEST ///



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


