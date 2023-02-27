
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>

#define MAXDAYS 10000

const int MAXLINES = 2800;
const int TIMEPERIOD = 14;
const int HAMMERSIZE = 3;   // (max(open,close) - low) x times |open - close|
const int DOJISIZE = 10;    // (high - low) x times |open - close|
const int DOJIEXTREMES = 3; // top 1/x and bottom 1/x defined as dragon-fly & gravestone respectively

/************************************** DAY FEATURES **********************************************/

typedef struct day
{

    // Basic day features:
    char date[12];
    float open;
    float close;
    float high;
    float low;
    float vol;

    // RSI indicators:
    float avgUp;
    float avgDown;
    float change;
    float RSI;

    // Japanese candlesticks indicators:
    bool bullEngulf;
    bool bullRelEngulf;
    bool bearEngulf;
    bool bearRelEngulf;
    bool morningStar;
    bool eveningStar;
    bool hammer;
    bool llDoji;
    bool dfDoji;
    bool gsDoji;

    // "Consecutive disagreements" indicators:
    bool disagreement;
    bool consDisagreement;

} day_t;


/************************************** INPUT & OUTPUT PROCESSING **********************************************/

void resetBuffer(char *buffer)
{
    for (int i = strlen(buffer); i >= 0; i--)
    {
        buffer[i] = '\0';
    }
}


void parseFile(char *fileDirectory, day_t *dayArray[])
{

    day_t *tempArray[MAXDAYS];

    FILE *fp = fopen(fileDirectory, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening %s\n", fileDirectory);
        return;
    }

    char c;
    char line[1024];
    int i;
    int numLine = 0;
    int d = 0;

    while (numLine < MAXLINES)
    {

        i = 0;

        while ((c = fgetc(fp)) != '\n')
        {
            if (c == 'M')
            {
                while ((c = fgetc(fp)) != '\n')
                {
                }
                break;
            }
            else if (c == ',')
            {
                continue;
            }
            else if (c == '"')
            {
                line[i] = ' ';
            }
            else
            {
                line[i] = c;
            }
            i++;
        }

        if (numLine != 0)
        {
            day_t *day = malloc(sizeof(day_t));
            // Initialize standard parameters
            sscanf(line, "%s %f %f %f %f %f", (char *)&day->date, &day->close, &day->open, &day->high, &day->low, &day->vol);
            // Initialize analysis parameters to false:
            day->disagreement = false;
            day->consDisagreement = false;
            day->bearEngulf = false;
            day->bullEngulf = false;
            day->morningStar = false;
            day->eveningStar = false;
            day->hammer = false;
            day->llDoji = false;
            day->dfDoji = false;
            day->gsDoji = false;
            tempArray[d] = day;
            d++;
        }

        resetBuffer(line);
        numLine++;
    }

    fclose(fp);

    // Construct array containing all days:
    for (int i = 0; i < d; i++)
    {
        dayArray[i] = tempArray[d - 1 - i];
    }
}


void printDay(day_t *day)
{

    printf("%s %f %f %f %f %f   %f\n", (char *)day->date, day->close, day->open, day->high, day->low, day->vol, day->RSI);
}


void printDays(day_t *dayArray[])
{

    for (int i = 0; i < MAXLINES - 1; i++)
    {
        if (i == 0)
        {
            printf("Date         Close         Open        High         Low          Volume        RSI\n");
        }
        day_t *day = dayArray[i];
        printDay(day);
    }
}


/************************************** RSI **********************************************/

void computeWilderRSI(day_t *dayArray[])
{

    float totalUp = 0;
    float totalDown = 0;

    for (int d = 1; d < MAXLINES - 1; d++)
    {

        day_t *day = dayArray[d];
        day_t *prev = dayArray[d - 1];

        day->change = day->close - prev->close;

        // Computing RSI starting values:
        if (d == TIMEPERIOD)
        {

            for (int i = (d + 1) - TIMEPERIOD; i <= d; i++)
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

            day->avgUp = (float)(totalUp / TIMEPERIOD);
            day->avgDown = (float)(totalDown / TIMEPERIOD);
        }

        // Computing RSI for successive values:
        else if (d > TIMEPERIOD)
        {

            float change = day->change;

            day->avgUp = (float)(((TIMEPERIOD - 1) * (prev->avgUp) + (change > 0) * (change)) / TIMEPERIOD);
            day->avgDown = (float)(((TIMEPERIOD - 1) * (prev->avgDown) - (change < 0) * (change)) / TIMEPERIOD);
        }

        day->RSI = 100 - (float)(100 / (1 + (float)(day->avgUp / day->avgDown)));
    }
}


/************************************** JAPANESE CANDLESTICK INDICATORS **********************************************/

void recordEngulfments(day_t *dayArray[])
{

    // Iterate through all days:
    for (int i = 0; i < MAXLINES - 2; i++)
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


void recordStars(day_t *dayArray[])
{

    // Iterate through all days:
    for (int i = 2; i < MAXLINES - 1; i++)
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


void recordHammers(day_t *dayArray[], const int hammerSize)
{

    // Iterate through all days:
    for (int i = 0; i < MAXLINES - 1; i++)
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


void recordDojis(day_t *dayArray[], const int dojiSize, const int dojiExtremes)
{

    // Iterate through all days:
    for (int i = 0; i < MAXLINES - 1; i++)
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


/************************************** ALGORITHMS **********************************************/

/******************* "Consecutive RSI Disagreements" ********************/

const int disInterval = 2; // interval between 2 points over which a disagreement is defined
const int consecDis = 3;   // total consecutive disagreements
const int disDist = 7;     // distance between two divegences to be defined as consecutive

void recordConsDisagreements(day_t *dayArray[])
{

    // Backtrack array keeping track of all previously disagreeing days:
    int divArray[MAXLINES];
    int p = 0;
    bool isCons;

    // Iterate through all days:
    for (int i = TIMEPERIOD + 1; i < MAXLINES - 1; i++)
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


/************************************** MAIN & TESTING **********************************************/

int main()
{

    day_t *dayArray[MAXDAYS];

    parseFile("./data/DAX Historical Data.csv", dayArray);

    computeWilderRSI(dayArray);
    recordConsDisagreements(dayArray);

    recordEngulfments(dayArray);
    recordStars(dayArray);
    recordHammers(dayArray, HAMMERSIZE);
    recordDojis(dayArray, DOJISIZE, DOJIEXTREMES);

    // printDays(dayArray);

    return 0;
}
