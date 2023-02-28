
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "day.h"
#include "dataProcessing.h"


/*********************** GRAPH MAIN PARAMETERS ***********************************/

const char* STARTDATE = "08/08/2022";
const char* ENDDATE = "02/24/2023";
const char* DATA = "../data/DAX Historical Data.csv";

/*********************** DATA DIRECTORIES ***********************************/

const char* datesDirectory = "../graphs/data/dates.txt";
const char* closesDirectory = "../graphs/data/closes.txt";
const char* volumesDirectory = "../graphs/data/volumes.txt";
const char* RSIDirectory = "../graphs/data/RSI.txt";

const char* bullEngulfDirectory = "../graphs/data/bullEngulf.txt";
const char* bullRelEngulfDirectory = "../graphs/data/bullRelEngulf.txt";
const char* bearEngulfDirectory = "../graphs/data/bearEngulf.txt";
const char* bearRelEngulfDirectory = "../graphs/data/bearRelEngulf.txt";
const char* morningStarDirectory = "../graphs/data/morningStar.txt";
const char* eveningStarDirectory = "../graphs/data/eveningStar.txt";
const char* hammerDirectory = "../graphs/data/hammer.txt";
const char* llDojiDirectory = "../graphs/data/llDoji.txt";
const char* dfDojiDirectory = "../graphs/data/dfDoji.txt";
const char* gsDojiDirectory = "../graphs/data/gsDoji.txt";

const char* disagreementDirectory = "../graphs/data/disagreement.txt";
const char* consDisagreementDirectory = "../graphs/data/consDisagreement.txt";

/************************************** ANALYSIS PARAMETERS **********************************************/

const int TIMEPERIOD = 14;  // RSI time-period
const int HAMMERSIZE = 3;   // (max(open,close) - low) x times |open - close|
const int DOJISIZE = 10;    // (high - low) x times |open - close|
const int DOJIEXTREMES = 3; // top 1/x and bottom 1/x defined as dragon-fly & gravestone respectively



/************************************** MAIN & TESTING **********************************************/

int main()
{
    // Initializing data from file (dataProcessing.c):
    day_t *dayArray[MAXDAYS];
    const int DAYSRECORDED = parseFile(DATA, dayArray, STARTDATE, ENDDATE);
    if (DAYSRECORDED == 0){
        fprintf(stderr, "Error parsing file\n");
        exit(1);
    }

    // Computing analysis tools (tools.c):
    recordEngulfments(dayArray, DAYSRECORDED);
    recordStars(dayArray, DAYSRECORDED);
    recordHammers(dayArray, DAYSRECORDED, HAMMERSIZE);
    recordDojis(dayArray, DAYSRECORDED, DOJISIZE, DOJIEXTREMES);

    computeWilderRSI(dayArray, DAYSRECORDED, TIMEPERIOD);
    recordConsDisagreements(dayArray, DAYSRECORDED, TIMEPERIOD);

    printDates(dayArray, DAYSRECORDED, datesDirectory);

    printFloatAttributes(dayArray, DAYSRECORDED, "close", closesDirectory);
    printFloatAttributes(dayArray, DAYSRECORDED, "vol", volumesDirectory);
    printFloatAttributes(dayArray, DAYSRECORDED, "RSI", RSIDirectory);

    printBoolAttributes(dayArray, DAYSRECORDED, "bullEngulf", bullEngulfDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bullRelEngulf", bullRelEngulfDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bearEngulf", bearEngulfDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bearRelEngulf", bearRelEngulfDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "morningStar", morningStarDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "eveningStar", eveningStarDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "hammer", hammerDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "llDoji", llDojiDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "dfDoji", dfDojiDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "gsDoji", gsDojiDirectory);

    printBoolAttributes(dayArray, DAYSRECORDED, "disagreement", disagreementDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "consDisagreement", consDisagreementDirectory);






    // Printing days (dataProcessing.c):
    // printDays(dayArray, DAYSRECORDED);

    return 0;
}
