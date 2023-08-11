
#include "Indicators.h"

/******* INDICATOR CLASSES ********/

void Indicators::RSI::computeIndicatorBar(int& d)
{
    indicatorArray[d]->change = dp->getBar(d)->close() - dp->getBar(d-1)->close();
    // Computing RSI starting values:
    if (d == timePeriod) {
        double totalUp = 0;
        double totalDown = 0;
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

void Indicators::LocalMin::computeIndicatorBar(int& d)
{
    int leftDepth  = 0;
    int rightDepth = 0;
    int i = d; // Initialize iterator to d
    
    // Start iterating from the left side
    while (i < dp->getnumBars() && 
        (this->m) * dp->getBar(i-1)->close() > (this->m) * dp->getBar(i)->close()) { 
        leftDepth++;
        i++; 
    }
    // If we are still ascending/descending, then return and DO NOT mark bars as visited
    if (i == dp->getnumBars()) return;
    if (leftDepth == 0) {
        d++;
        return;
    }
    // By this point we have identified a maximum/minimum (mark it)
    this->indicatorArray[i-1]->leftDepth = leftDepth;
    this->indicatorArray[i-1]->m = (int) this->m;
    this->indicatorArray[i-1]->leftChange = 
        (double)((-1)*(this->m)) * ((double)100) * (this->dp->getBar(i-1)->close() - this->dp->getBar(i-1-leftDepth)->close()) / this->dp->getBar(i-1)->close();
    // Start iterating towards the right side
    while (i < dp->getnumBars() && 
        (this->m) * dp->getBar(i-1)->close() < (this->m) * dp->getBar(i)->close()) { 
        rightDepth++;
        i++; 
    }
    // Mark maximum/minimum's right side
    this->indicatorArray[i-1-rightDepth]->rightDepth = rightDepth;
    this->indicatorArray[i-1-rightDepth]->rightChange = 
        ((double)(this->m)) * ((double)100) * (this->dp->getBar(i-1)->close() - this->dp->getBar(i-1-rightDepth)->close()) / this->dp->getBar(i-1)->close();
    // Mark bars as visited if we didn't run out of bars (maximum/minimum fully identified)
    if (i < dp->getnumBars()) {
        d = i;
    }
}

void Indicators::LocalMin::computeIndicator()
{
    this->computeIndicatorBar(this->curr_it_bar); 
}

void Indicators::Divergence::markDivergence(int leftBarIndex, int rightBarIndex, int m)
{
    this->indicatorArray[rightBarIndex]->leftBarIndex  = leftBarIndex;
    this->indicatorArray[rightBarIndex]->rightBarIndex = rightBarIndex;
    this->indicatorArray[rightBarIndex]->leftBar       = dp->getBar(leftBarIndex);
    this->indicatorArray[rightBarIndex]->rightBar      = dp->getBar(rightBarIndex);
    this->indicatorArray[rightBarIndex]->divPoints     = this->indicatorArray[leftBarIndex]->divPoints + 1;
    this->indicatorArray[rightBarIndex]->m             = m;
}

void Indicators::Divergence::computeLocalStat(class LocalMin * const LocalStat, int& d) 
{
    int m = LocalStat->getIndicatorBar(d)->m;
    // Test for local maximum/minimum
    if (!LocalStat->getIndicatorBar(d)->isPresent())
        return;
    // Iterate back until previous local maximum/minimum
    for (int i = minDivPeriod; i < maxDivPeriod; i++) {
        if ((d - i <= 0) || !LocalStat->getIndicatorBar(d - i)->isPresent())
            continue;
        // Test divergence conditions:
        if (m * dp->getBar(d)->close() < m * dp->getBar(d-i)->close() &&
            m * this->RSI->getIndicatorBar(d)->RSI > m * this->RSI->getIndicatorBar(d-i)->RSI)
        {
            markDivergence(d - i, d, m);
        }
        // Break loop as soon as we find first maximum/minimum, with or without divergence
        break; 
    }
}

void Indicators::LongDivergence::computeLocalStat(class LocalMin * const LocalStat, int& d)
{
    int m = LocalStat->getIndicatorBar(d)->m;
    // Test for local maximum/minimum
    if (!LocalStat->getIndicatorBar(d)->isPresent())
        return;
    // Start iterating back within given limit
    for (int i = std::max(1, d - minDivPeriod); 
             i > std::max(1, d - maxDivPeriod); i--) {
        // If we find any point higher/lower than current maximum/minimum,
        // or the previous point was already marked with a long divergence, exit iteration
        if (m * dp->getBar(i)->close() < m * dp->getBar(d)->close() ||
            this->getIndicatorBar(i+1)->isPresent())
            break;
        // Find another maximum/minimum in between and test divergence condition
        if (LocalStat->getIndicatorBar(i)->isPresent() &&
            m * dp->getBar(d)->close()       < m * dp->getBar(i)->close() &&
            m * RSI->getIndicatorBar(d)->RSI > m * RSI->getIndicatorBar(i)->RSI)
        {
            markDivergence(i, d, m);
            // Break loop as soon as we find first divergence
            break;
        }
    }
}

void Indicators::Divergence::computeIndicatorBar(int& d)
{ 
    int p = d - 1; // work on previous bar
    this->computeLocalStat(this->LocalMin, p);
    this->computeLocalStat(this->LocalMax, p);
    // Compute divergence percentage change per bar if divergence present
    if (this->getIndicatorBar(p)->isPresent()) {
        double lc    = this->getIndicatorBar(p)->leftBar->close();
        double rc    = this->getIndicatorBar(p)->rightBar->close();
        double bars  = this->getIndicatorBar(p)->rightBarIndex - this->getIndicatorBar(p)->leftBarIndex;

        double pcpb = (double) 100 * (rc - lc) / (lc * bars);
        this->getIndicatorBar(p)->abs_perc_change_per_bar = pcpb > 0 ? pcpb : -pcpb;
    }
}

void Indicators::BollingerBands::computeIndicatorBar(int& d)
{
    // Compute average, or bollMiddle value:
    int i = d;
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

void Indicators::JCandleSticks::computeIndicatorBar(int& d)
{
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


