
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "day.h"
#include "dataProcessing.h"


/*********************** INPUT & OUTPUT PROCESSING PARAMETERS (dataProcessing) ***********************************/

const char* STARTDATE = "08/24/2022";
const char* ENDDATE = "02/24/2023";
const char* DATA = "../data/DAX Historical Data.csv";

const char* datesFile = "../graphs/dates.txt";
const char* closesFile = "../graphs/closes.txt";
const char* volumesFile = "../graphs/volumes.txt";
const char* RSIFile = "../graphs/RSI.txt";


/************************************** ANALYSIS PARAMETERS (tools) **********************************************/

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

    printDates(dayArray, DAYSRECORDED, datesFile);
    printCloses(dayArray, DAYSRECORDED, closesFile);
    printVolumes(dayArray, DAYSRECORDED, volumesFile);
    printRSI(dayArray, DAYSRECORDED, RSIFile);







    // Printing days (dataProcessing.c):
    // printDays(dayArray, DAYSRECORDED);

    return 0;
}
