
#include "Indicators.h"

#include <math.h>

/******* INDICATOR CLASSES ********/

void Indicators::RSI::computeIndicator()
{
    double totalUp = 0;
    double totalDown = 0;
    for (int d = 1; d < dp->getnumBars(); d++) {
        indicatorArray[d]->change = dp->getBar(d)->close() - dp->getBar(d-1)->close();
        // Computing RSI starting values:
        if (d == timePeriod) {
            for (int i = (d + 1) - timePeriod; i <= d; i++) {
                if (indicatorArray[i]->change > 0) 
                    totalUp += indicatorArray[i]->change; 
                else
                    totalDown -= indicatorArray[i]->change;
            }
            indicatorArray[d]->avgUp = (double)(totalUp / timePeriod);
            indicatorArray[d]->avgDown = (double)(totalDown / timePeriod);
            indicatorArray[d]->RSI = 100 - (double)(100 / (1 + (double)(indicatorArray[d]->avgUp / indicatorArray[d]->avgDown)));
        }
        // Computing RSI for successive values:
        else if (d > timePeriod) {
            double change = indicatorArray[d]->change;
            indicatorArray[d]->avgUp = (double)(((timePeriod - 1) * (indicatorArray[d-1]->avgUp) + (change > 0) * (change)) / timePeriod);
            indicatorArray[d]->avgDown = (double)(((timePeriod - 1) * (indicatorArray[d-1]->avgDown) - (change < 0) * (change)) / timePeriod);
            indicatorArray[d]->RSI = 100 - (double)(100 / (1 + (double)(indicatorArray[d]->avgUp / indicatorArray[d]->avgDown)));
        }
    }
}

void Indicators::LocalMin::computeIndicator()
{
    int d = 1;
    int leftDepth;
    int rightDepth;
    while (d < dp->getnumBars()) {
        leftDepth = 0;
        while (d < dp->getnumBars() && 
            (this->m) * dp->getBar(d-1)->close() > (this->m) * dp->getBar(d)->close()) 
        { 
            leftDepth++;
            d++; 
        }
        if (leftDepth != 0) {
            this->indicatorArray[d-1]->leftDepth = leftDepth;
            this->indicatorArray[d-1]->m = (int) this->m;
            this->indicatorArray[d-1]->leftChange = 
                (double)((-1)*(this->m)) * ((double)100) * (this->dp->getBar(d-1)->close() - this->dp->getBar(d-1-leftDepth)->close()) / this->dp->getBar(d-1)->close();
            rightDepth = 0;
            while (d < dp->getnumBars() && 
                (this->m) * dp->getBar(d-1)->close() < (this->m) * dp->getBar(d)->close()) 
            { 
                rightDepth++;
                d++; 
            }
            this->indicatorArray[d-1-rightDepth]->rightDepth = rightDepth;
            this->indicatorArray[d-1-rightDepth]->rightChange = 
                ((double)(this->m)) * ((double)100) * (this->dp->getBar(d-1)->close() - this->dp->getBar(d-1-rightDepth)->close()) / this->dp->getBar(d-1)->close();
        } else { 
            d++; 
        }
    }
}

Indicators::Divergence::Divergence(Bars* dp,
                                class LocalMax* LocalMax, 
                                class LocalMin* LocalMin, 
                                class RSI* RSI,
                                int minDivPeriod, 
                                int maxDivPeriod,
                                const std::string name)
    : Indicator(dp, name), minDivPeriod(minDivPeriod), maxDivPeriod(maxDivPeriod)
{
    this->LocalMax = LocalMax;
    this->LocalMin = LocalMin;
    this->RSI = RSI;
}

void Indicators::Divergence::computeIndicator()
{ 
    // Iterate through each Bar:
    for (int d = minRSItimePeriods * this->RSI->timePeriod; d < dp->getnumBars(); d++) {  
        // Find a local maximum
        if (this->LocalMax->getIndicatorBar(d)->isPresent()) {
            // Iterate back until previous local maximum
            for (int i = minDivPeriod; i < maxDivPeriod; i++) {
                if ((d-i > 0) && this->LocalMax->getIndicatorBar(d-i)->isPresent()) {
                    // Test divergence conditions:
                    if (dp->getBar(d)->close()             > dp->getBar(d-i)->close()
                        && this->RSI->getIndicatorBar(d)->RSI < this->RSI->getIndicatorBar(d-i)->RSI)
                    {
                        this->indicatorArray[d]->leftBarIndex   = d-i;
                        this->indicatorArray[d]->rightBarIndex  = d;
                        this->indicatorArray[d]->leftBar   = dp->getBar(d-i);
                        this->indicatorArray[d]->rightBar  = dp->getBar(d);
                        this->indicatorArray[d]->divPoints = this->indicatorArray[d-i]->divPoints + 1;
                        this->indicatorArray[d]->m         = -1;
                    }
                    break;
                }
            }
        }
        // Find a local minimum
        if (this->LocalMin->getIndicatorBar(d)->isPresent()) {
            // Iterate back until previous local minimum
            for (int i = minDivPeriod; i < maxDivPeriod; i++) {
                if ((d-i > 0) && this->LocalMin->getIndicatorBar(d-i)->isPresent()) {
                    // Test divergence conditions:
                    if (dp->getBar(d)->close()             < dp->getBar(d-i)->close()
                        && this->RSI->getIndicatorBar(d)->RSI > this->RSI->getIndicatorBar(d-i)->RSI)
                    {
                        this->indicatorArray[d]->leftBarIndex   = d-i;
                        this->indicatorArray[d]->rightBarIndex  = d;
                        this->indicatorArray[d]->leftBar   = dp->getBar(d-i);
                        this->indicatorArray[d]->rightBar  = dp->getBar(d);
                        this->indicatorArray[d]->divPoints = this->indicatorArray[d-i]->divPoints + 1;
                        this->indicatorArray[d]->m         = 1;
                    }
                    break;
                }
            }
        }
    }
}

void Indicators::LongDivergence::computeIndicator()
{
    // Iterate through each Bar:
    for (int d = minRSItimePeriods*this->RSI->timePeriod; d < dp->getnumBars(); d++) {  
        // Find a local maximum
        if (LocalMax->getIndicatorBar(d)->isPresent()) {
            // Start iterating back within given limit
            for (int i = std::max(1, d - minDivPeriod); 
                     i > std::max(1, d - maxDivPeriod); 
                     i--) {
                // If we find any point greater than current maximum,
                // or the previous point was marked with a divergence, exit
                if (dp->getBar(i)->close() > dp->getBar(d)->close()
                  || this->getIndicatorBar(i+1)->isPresent())
                    break;
                // Find another max in between and test divergence condition
                if (LocalMax->getIndicatorBar(i)->isPresent()) {
                    if (dp->getBar(d)->close()          > dp->getBar(i)->close()
                        && RSI->getIndicatorBar(d)->RSI < RSI->getIndicatorBar(i)->RSI)
                    {
                        this->indicatorArray[d]->leftBarIndex   = i;
                        this->indicatorArray[d]->rightBarIndex  = d;
                        this->indicatorArray[d]->leftBar   = dp->getBar(i);
                        this->indicatorArray[d]->rightBar  = dp->getBar(d);
                        this->indicatorArray[d]->divPoints = this->indicatorArray[i]->divPoints + 1;
                        this->indicatorArray[d]->m         = -1;
                        // Break loop if divergence is found
                        break;
                    }
                }
            }
        }
        // Find a local minimum
        if (LocalMin->getIndicatorBar(d)->isPresent()) {
            // Start iterating back within given limit
            for (int i = std::max(1, d - minDivPeriod); 
                     i > std::max(1, d - maxDivPeriod); i--) {
                // If we find any point lower than current minimum, 
                // or previous minimum was marked with a divergence, exit
                if (dp->getBar(i)->close() < dp->getBar(d)->close()
                  || this->getIndicatorBar(i+1)->isPresent())
                    break;
                // Find another min in between and test divergence condition
                if (LocalMin->getIndicatorBar(i)->isPresent())  {
                    if (dp->getBar(d)->close() < dp->getBar(i)->close()
                        && RSI->getIndicatorBar(d)->RSI > RSI->getIndicatorBar(i)->RSI)
                    {
                        this->indicatorArray[d]->leftBarIndex   = i;
                        this->indicatorArray[d]->rightBarIndex  = d;
                        this->indicatorArray[d]->leftBar   = dp->getBar(i);
                        this->indicatorArray[d]->rightBar  = dp->getBar(d);
                        this->indicatorArray[d]->divPoints = this->indicatorArray[i]->divPoints + 1;
                        this->indicatorArray[d]->m         = 1;
                        // Break loop if divergence is found
                        break;
                    }
                }
            }
        }
    }
}

Indicators::BollingerBands::BollingerBands(Bars* dp,
                                           double stDevUp, 
                                           double stDevDown, 
                                           int timePeriod, 
                                           const std::string name) 
    : Indicator(dp, name)
{
    this->stDevUp = stDevUp;
    this->stDevDown = stDevDown;
    this->timePeriod = timePeriod;
}

void Indicators::BollingerBands::computeIndicator()
{
    for (int i = this->timePeriod; i < dp->getnumBars(); i++) {
        // Compute average, or bollMiddle value:
        double midSum = 0;
        for (int j = 0; j < this->timePeriod; j++) {
            midSum += dp->getBar(i-j)->close();
        }
        this->indicatorArray[i]->bollMiddle = (double) (midSum / (double) this->timePeriod);
        // Compute standard deviation:
        double SdSum = 0;
        for (int j = 0; j < this->timePeriod; j++) {
            SdSum += powf((dp->getBar(i-j)->close() - this->indicatorArray[i]->bollMiddle), (double) 2);
        }
        double SD = sqrtf((double) (SdSum / (double) this->timePeriod));
        // Compute upper & lower bands:
        this->indicatorArray[i]->bollLower = this->indicatorArray[i]->bollMiddle - SD * this->stDevDown;
        this->indicatorArray[i]->bollUpper = this->indicatorArray[i]->bollMiddle + SD * this->stDevUp;
        //** CROSSOVERS **//
        if (i > this->timePeriod) {
            if (dp->getBar(i)->close()  > this->indicatorArray[i]->bollUpper
            && dp->getBar(i-1)->close() < this->indicatorArray[i-1]->bollUpper)
                this->indicatorArray[i]->crossUpperUp = true;
            if (dp->getBar(i)->close()  < this->indicatorArray[i]->bollUpper
            && dp->getBar(i-1)->close() > this->indicatorArray[i-1]->bollUpper)
                this->indicatorArray[i]->crossUpperDown = true;
            if (dp->getBar(i)->close()  > this->indicatorArray[i]->bollMiddle
            && dp->getBar(i-1)->close() < this->indicatorArray[i-1]->bollMiddle) 
                this->indicatorArray[i]->crossMiddleUp = true;
            if (dp->getBar(i)->close()  < this->indicatorArray[i]->bollMiddle
            && dp->getBar(i-1)->close() > this->indicatorArray[i-1]->bollMiddle) 
                this->indicatorArray[i]->crossMiddleDown = true;
            if (dp->getBar(i)->close()  > this->indicatorArray[i]->bollLower
            && dp->getBar(i-1)->close() < this->indicatorArray[i-1]->bollLower) 
                this->indicatorArray[i]->crossLowerUp = true;
            if (dp->getBar(i)->close()  < this->indicatorArray[i]->bollLower
            && dp->getBar(i-1)->close() > this->indicatorArray[i-1]->bollLower) 
                this->indicatorArray[i]->crossLowerDown = true;
        }
    }
}

Indicators::JCandleSticks::JCandleSticks(Bars* dp,
                                        double hammerSize, 
                                        double dojiSize, 
                                        int dojiExtremes,
                                        const std::string name) 
    : Indicator(dp, name)
{
    this->hammerSize = hammerSize;
    this->dojiSize = dojiSize;
    this->dojiExtremes = dojiExtremes;
}

void Indicators::JCandleSticks::computeIndicator()
{
    for (int d = 2; d < dp->getnumBars(); d++) {
        double currOpen = this->dp->getBar(d)->open();
        double currClose = this->dp->getBar(d)->close();
        double currLow = this->dp->getBar(d)->low();
        double currHigh = this->dp->getBar(d)->high();

        double prevOpen = this->dp->getBar(d-1)->open();
        double prevClose = this->dp->getBar(d-1)->close();
        double prevLow = this->dp->getBar(d-1)->low();
        double prevHigh = this->dp->getBar(d-1)->high();

        double prevprevOpen = this->dp->getBar(d-2)->open();
        double prevprevClose = this->dp->getBar(d-2)->close();
        //** ENGULFMENTS, HARAMI, PIERCING, DARK CLOUD **//
        if (prevOpen > prevClose     // Bearish candlestick
            && currOpen < currClose) // followed by bullish candlestick
        {
            if (currOpen < prevClose && currClose > prevOpen)   
                this->indicatorArray[d]->bullEngulf = true;
            if (currClose < prevOpen && currOpen > prevClose) 
                this->indicatorArray[d]->bullHarami = true;
            if (currOpen < prevLow 
             && currClose > (double)((prevOpen + prevClose)/ (double) 2)) 
                this->indicatorArray[d]->piercing = true;
        }
        if (prevOpen < prevClose     // Bullish candlestick
            && currOpen > currClose) // followed by bearish candlestick
        {
            if (currOpen > prevClose && currClose < prevOpen)  
                this->indicatorArray[d]->bearEngulf = true;
            if (currOpen < prevClose && currClose > prevOpen) 
                this->indicatorArray[d]->bearHarami = true;
            // Dark cloud criteria:
            if (currOpen > prevHigh
             && currClose < (double)((prevOpen + prevClose)/ (double) 2)) 
                this->indicatorArray[d]->darkCloud = true;
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
        if ((std::max(currOpen,currClose) - currLow) > this->hammerSize * (std::max(currOpen,currClose) - std::min(currOpen,currClose)) 
            && (std::max(currOpen,currClose) - std::min(currOpen,currClose)) > (currHigh - std::max(currOpen,currClose)))
        {
            this->indicatorArray[d]->hammer = true;
        }
        //** DOJIS **//
        if ((currHigh - currLow) > this->dojiSize * (std::max(currOpen,currClose) - std::min(currOpen,currClose)))
        { 
            if (std::min(currOpen,currClose) > currLow + (currHigh - currLow) * ((1 - (double)1 / (double) this->dojiExtremes))) 
                this->indicatorArray[d]->dfDoji = true;
            else if (std::max(currOpen,currClose) < currLow + (currHigh - currLow) * ((double)1 / (double) this->dojiExtremes))
                this->indicatorArray[d]->gsDoji = true;
            else
                this->indicatorArray[d]->llDoji = true;
        }
    }
}


/// TEST ///

namespace TestDivConditions
{
    class LocalStat
    {

        int LleftDepth = 0;
        double LleftChange = 0.0;
        int LrightDepth = 0;
        double LrightChange = 0.0;

        int RleftDepth = 0;
        double RleftChange = 0.0;
        int RrightDepth = 0;
        double RrightChange = 0.0;

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


