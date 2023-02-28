
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "day.h"


/************************ TECHNICAL ANALYSIS TOOLS ***************************/

/************************************** RSI **********************************************/

void computeWilderRSI(day_t *dayArray[], const int daysRecorded, const int timePeriod)
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

void recordEngulfments(day_t *dayArray[], const int daysRecorded)
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
        }
    }
}


void recordStars(day_t *dayArray[], const int daysRecorded)
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


void recordHammers(day_t *dayArray[], const int daysRecorded, const int hammerSize)
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


void recordDojis(day_t *dayArray[], const int daysRecorded, const int dojiSize, const int dojiExtremes)
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




/**************** ALGORITHMS / COMPOSITE TOOLS *********************/

/******************* "Consecutive RSI Disagreements" ********************/

const int disInterval = 2; // interval between 2 points over which a disagreement is defined
const int consecDis = 3;   // total consecutive disagreements
const int disDist = 7;     // distance between two divegences to be defined as consecutive

void recordConsDisagreements(day_t *dayArray[], const int daysRecorded, const int timePeriod)
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