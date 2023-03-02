
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <math.h>

#include "day.h"


/************************ TECHNICAL ANALYSIS TOOLS ***************************/

/************************************** RSI **********************************************/

void 
computeWilderRSI(day_t *dayArray[], const int daysRecorded, const int timePeriod)
{

    float totalUp = 0;
    float totalDown = 0;

    for (int d = 1; d < daysRecorded; d++)
    {

        day_t *day = dayArray[d];
        day_t *prev = dayArray[d - 1];

        day->change = day->close - prev->close;

        // Computing RSI starting values:
        if (d == timePeriod)
        {
            for (int i = (d + 1) - timePeriod; i <= d; i++)
            {
                if (dayArray[i]->change > 0)
                {
                    totalUp += dayArray[i]->change;
                }
                else
                {
                    totalDown -= dayArray[i]->change;
                }
            }

            day->avgUp = (float)(totalUp / timePeriod);
            day->avgDown = (float)(totalDown / timePeriod);

            day->RSI = 100 - (float)(100 / (1 + (float)(day->avgUp / day->avgDown)));
        }

        // Computing RSI for successive values:
        else if (d > timePeriod)
        {
            float change = day->change;

            day->avgUp = (float)(((timePeriod - 1) * (prev->avgUp) + (change > 0) * (change)) / timePeriod);
            day->avgDown = (float)(((timePeriod - 1) * (prev->avgDown) - (change < 0) * (change)) / timePeriod);

            day->RSI = 100 - (float)(100 / (1 + (float)(day->avgUp / day->avgDown)));
        }
    }
}


/************************************** JAPANESE CANDLESTICK INDICATORS **********************************************/

void 
recordEngulfments(day_t *dayArray[], const int daysRecorded)
{

    // Iterate through all days:
    for (int i = 0; i < daysRecorded -1; i++)
    {
        day_t *day = dayArray[i];
        day_t *next = dayArray[i + 1];

        float currOpen = day->open;
        float currClose = day->close;
        float currMin = day->low;
        float currMax = day->high;
        float nextOpen = next->open;
        float nextClose = next->close;
        float nextMin = next->low;
        float nextMax = next->high;

        // Bullish engulfment selection criteria:
        if (currOpen > currClose     // Bearish candlestick
            && nextOpen < nextClose) // followed by bullish candlestick
        {
            // Strict engulfment criteria:
            if (nextOpen < currMin       // next candlestick opens below current candlesticks's MINIMUM
                && nextClose > currMax)  // next candlestick closes above current candlestick's MAXIMUM
            { 
                next->bullEngulf = true;
                next->bullRelEngulf = true;
            }

            // Relaxed engulfment criteria:
            else if (nextOpen < currClose // next candlestick opens below current candlesticks's CLOSE
                && nextClose > currOpen)  // next candlestick closes above current candlestick's OPEN
            { 
                next->bullRelEngulf = true;
            }

            // Strict harami criteria:
            if ( nextMax < currOpen
                && nextMin > currClose)
            {
                next->bullHarami = true;
                next->bullRelHarami = true;
            }

            // Relaxed harami criteria:
            else if ( nextClose < currOpen
                && nextOpen > currClose)
            {
                next->bullRelHarami = true;
            }
        }


        // Bearish engulfment selection criteria:
        if (currOpen < currClose     // Bullish candlestick
            && nextOpen > nextClose) // followed by bearish candlestick
        {
            // Strict engulfment criteria
            if (nextOpen > currMax       // next candlestick opens above current candlesticks's MAXIMUM
                && nextClose < currMin)  // next candlestick closes below current candlestick's MINIMUM
            { 
                next->bearEngulf = true;
                next->bearRelEngulf = true;
            }

            // Relaxed engulfment criteria:
            else if (nextOpen > currClose // next candlestick opens above current candlesticks's CLOSE
                && nextClose < currOpen)  // next candlestick closes below current candlestick's OPEN
            { 
                next->bearRelEngulf = true;
            }

            // Strict harami criteria:
            if ( nextMax < currClose
                && nextMin > currOpen)
            {
                next->bearHarami = true;
                next->bearRelHarami = true;
            }

            // Relaxed harami criteria:
            else if ( nextOpen < currClose
                && nextClose > currOpen)
            {
                next->bearRelHarami = true;
            }
        }
    }
}


void 
recordStars(day_t *dayArray[], const int daysRecorded)
{

    // Iterate through all days:
    for (int i = 2; i < daysRecorded; i++)
    {
        day_t *d1 = dayArray[i - 2];
        day_t *d2 = dayArray[i - 1];
        day_t *d3 = dayArray[i];

        float open1 = d1->open;
        float close1 = d1->close;
        float open2 = d2->open;
        float close2 = d2->close;
        float open3 = d3->open;
        float close3 = d3->close;

        // Morning star selection criteria:
        if (open1 > close1                      // first bearish candlestick
            && open3 < close3                   // third bullish candlestick
            && close1 > open2                   // second day opens and closes below close of first day
            && close1 > close2 && open3 > open2 // second day opens and closes below open of third day
            && open3 > close2)
        {
            // Mark third day:
            d3->morningStar = true;
        }

        // Evening star selection criteria:
        if (open1 < close1                      // first bullish candlestick
            && open3 > close3                   // third bearish candlestick
            && close1 < open2                   // second day opens and closes above close of first day
            && close1 < close2 && open3 < open2 // second day opens and closes above open of third day
            && open3 < close2)
        {
            // Mark third day:
            d3->eveningStar = true;
        }
    }
}


void 
recordHammers(day_t *dayArray[], const int daysRecorded, const int hammerSize)
{

    // Iterate through all days:
    for (int i = 0; i < daysRecorded; i++)
    {

        day_t *day = dayArray[i];
        float open = day->open;
        float close = day->close;
        float low = day->low;
        float high = day->high;
        float diff = open - close;

        float max;
        float min;
        if (open > close)
        {
            max = open;
            min = close;
        }
        else
        {
            max = close;
            min = open;
        }

        // Unmark hammer variable if previously assigned:
        day->hammer = false;

        // Hammer selection citeria:
        if ((max - low) > hammerSize * (max - min) && (max - min) > (high - max))
        {
            day->hammer = true;
        }
    }
}


void 
recordDojis(day_t *dayArray[], const int daysRecorded, const int dojiSize, const int dojiExtremes)
{

    // Iterate through all days:
    for (int i = 0; i < daysRecorded; i++)
    {

        day_t *day = dayArray[i];
        float open = day->open;
        float close = day->close;
        float low = day->low;
        float high = day->high;
        float diff = open - close;

        float max;
        float min;
        if (open > close)
        {
            max = open;
            min = close;
        }
        else
        {
            max = close;
            min = open;
        }

        // Unmark any previous doji assignments:
        day->llDoji = false;
        day->dfDoji = false;
        day->gsDoji = false;

        // Doji selection citeria:
        if ((high - low) > dojiSize * (max - min))
        { // General doji selection:

            // Dragon-fly doji criteria:
            if (min > low + (high - low) * ((1 - (float)1 / dojiExtremes)))
            {
                day->dfDoji = true;
            }

            // Gravestone doji criteria:
            else if (max < low + (high - low) * ((float)1 / dojiExtremes))
            {
                day->gsDoji = true;
            }

            else
            {
                day->llDoji = true;
            }
        }
    }
}


/************************ MACD INDICATORS ***************************/

void 
recordMACD(day_t *dayArray[], const int daysRecorded, const int lower, const int upper)
{
    for (int i = upper; i < daysRecorded; i++)
    {
        // Computing upper and lower sums:
        float upperSum = 0;
        float lowerSum = 0;
        for (int j = 0; j < upper; j++)
        {
            if (j < lower) { lowerSum += dayArray[i-j] -> close; }
            upperSum += dayArray[i-j] -> close;
        }

        // Computing MACD:
        dayArray[i] -> MACD = (float)(lowerSum / (float)lower) - (float)(upperSum / (float)upper);
    }
}


void 
recordSigMACD(day_t *dayArray[], const int daysRecorded, const int MACDupper, const int avgPeriod)
{
    for (int i = MACDupper + avgPeriod; i < daysRecorded; i++)
    {
        // Computing MACD sums:
        float MACDSum = 0;
        for (int j = 0; j < avgPeriod; j++)
        {
            MACDSum += dayArray[i-j] -> MACD;
        }

        // Computing MACD signal line:
        dayArray[i] -> sigMACD = (float) (MACDSum / (float) avgPeriod);
    }
}


void 
recordSigBuySell(day_t *dayArray[], const int daysRecorded, const int totPeriod)
{
    for (int i = totPeriod + 1; i < daysRecorded; i++)
    {
        float currMACD = dayArray[i] -> MACD;
        float prevMACD = dayArray[i-1] -> MACD;
        float currSig = dayArray[i] -> sigMACD;
        float prevSig = dayArray[i-1] -> sigMACD;

        // Buy signal if MACD rises above signal line:
        if (prevMACD < prevSig && currMACD > currSig) { dayArray[i] -> MACDbuySig = true; }
        
        // Sell signal if MACD falls below signal line:
        if (prevMACD > prevSig && currMACD < currSig) { dayArray[i] -> MACDsellSig = true; }
    }
}


/************************************** BOLLINGER BANDS **********************************************/

void 
recordBollingerBands(day_t *dayArray[], const int daysRecorded, const int timePeriod, float standardDeviations)
{
    // Set all bands to closing price by default before timePeriod has elapsed:
    for (int i = 0; i < timePeriod; i++)
    {
        dayArray[i] -> bollMiddle = dayArray[i] -> close;
        dayArray[i] -> bollUpper = dayArray[i] -> close;
        dayArray[i] -> bollLower = dayArray[i] -> close;
    }

    for (int i = timePeriod; i < daysRecorded; i++)
    {
        // Compute average, or bollMiddle value:
        float middleSum = 0;
        for (int j = 0; j < timePeriod; j++)
        {
            middleSum += dayArray[i-j] -> close;
        }
        dayArray[i] -> bollMiddle = (float) (middleSum / (float) timePeriod);

        // Compute standard deviation:
        float SdSum = 0;
        for (int j = 0; j < timePeriod; j++)
        {
            SdSum += powf((dayArray[i-j]->close - dayArray[i]->bollMiddle), (float) 2);
        }
        float SD = sqrtf((float) (SdSum / (float) timePeriod));

        // Compute upper & lower bands:
        dayArray[i] -> bollLower = dayArray[i] -> bollMiddle - standardDeviations * SD;
        dayArray[i] -> bollUpper = dayArray[i] -> bollMiddle + standardDeviations * SD;
    }
}

void
recordBollingerSignals(day_t *dayArray[], const int daysRecorded, const int timePeriod)
{
    for (int i = timePeriod + 1; i < daysRecorded; i++)
    {
        float currClose = dayArray[i] -> close;
        float prevClose = dayArray[i-1] -> close;
        float currUpper = dayArray[i] -> bollUpper;
        float prevUpper = dayArray[i-1] -> bollUpper;
        float currLower = dayArray[i] -> bollLower;
        float prevLower = dayArray[i-1] -> bollLower;

        // If close cuts above lower band, buy signal:
        if (prevLower > prevClose && currLower < currClose) { dayArray[i] -> bollBuySig = true; }

        // If close cuts below upper band, sell signal:
        if (prevUpper < prevClose && currUpper > currClose) { dayArray[i] -> bollSellSig = true; }
    }
}

/**************** ALGORITHMS / COMPOSITE TOOLS *********************/

/******************* "Consecutive RSI Disagreements" ********************/

void 
recordConsDisagreements(day_t *dayArray[], const int daysRecorded, const int timePeriod, const int disInterval, const int consecDis, const int disDist)
{

    // Backtrack array keeping track of all previously disagreeing days:
    int divArray[daysRecorded];
    int p = 0;
    bool isCons;

    // Iterate through all days:
    for (int i = timePeriod + 1; i < daysRecorded; i++)
    {
        day_t *day = dayArray[i];
        day_t *prev = dayArray[i - disInterval];

        // Disagreement selection criteria:
        // Trend disagreement required between disInterval days:
        if ((day->RSI - prev->RSI > 0) != (day->close - prev->close > 0))
        {

            //  Mark day as divergent (only depends on disInterval)
            day->disagreement = true;
            divArray[p] = i;

            if (p > consecDis - 2)
            {
                // Consecutive disagreement selection criteria (depends on cosecDiv & disDist):
                // consecDis total disagreements less than disDist days apart each.
                // Verify distance to previous divergent days:
                isCons = true;
                for (int j = 0; j < consecDis - 1; j++)
                {
                    if (divArray[p - j] - divArray[p - j - 1] > disDist)
                    {
                        isCons = false;
                    }
                }

                if (isCons)
                {
                    // Record (consecDis)th (i.e. final) day of consecutive disagreements as consecutively "divergent":
                    day->consDisagreement = true;
                }
            }
            // Increment index of disagreeing days array:
            p++;
        }
    }
}
