
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "day.h"
#include "dataProcessing.h"


/*********************** GRAPH MAIN PARAMETERS ***********************************/

const char* STARTDATE = "08/24/2022";
const char* ENDDATE = "02/24/2023";
const char* DATA = "../data/DAX Historical Data.csv";


/****************** ANALYSIS PARAMETERS ******************************/

const int TIMEPERIOD = 14;  // RSI time-period
const int HAMMERSIZE = 3;   // (max(open,close) - low) x times |open - close|
const int DOJISIZE = 10;    // (high - low) x times |open - close|
const int DOJIEXTREMES = 3; // top 1/x and bottom 1/x defined as dragon-fly & gravestone respectively
const int UPPERAVG = 26;    // MACD upper moving average
const int LOWERAVG = 12;    // MACD lower moving average
const int SIGAVG = 9;       // timeperiod for MACD moving average signal line
const int BOLLPERIOD = 20;  // Moving average period for Bollinger bands
const float BOLLSD = 2;     // Number of standard deviations for Bollinger bands


/**************** ALGORITHMS / COMPOSITE TOOLS *********************/

/******************* "Consecutive RSI Disagreements" ********************/

const int disInterval = 2; // interval between 2 points over which a disagreement is defined
const int consecDis = 3;   // total consecutive disagreements
const int disDist = 7;    // distance between two divegences to be defined as consecutive

/*********************** DATA DIRECTORIES ***********************************/

const char* datesDirectory = "../graphs/data/dates.txt";
const char* closesDirectory = "../graphs/data/closes.txt";
const char* volumesDirectory = "../graphs/data/volumes.txt";
const char* RSIDirectory = "../graphs/data/RSI.txt";

const char* bullEngulfDirectory = "../graphs/data/bullEngulf.txt";
const char* bullRelEngulfDirectory = "../graphs/data/bullRelEngulf.txt";
const char* bearEngulfDirectory = "../graphs/data/bearEngulf.txt";
const char* bearRelEngulfDirectory = "../graphs/data/bearRelEngulf.txt";
const char* bullHaramiDirectory = "../graphs/data/bullHarami.txt";
const char* bullRelHaramiDirectory = "../graphs/data/bullRelHarami.txt";
const char* bearHaramiDirectory = "../graphs/data/bearHarami.txt";
const char* bearRelHaramiDirectory = "../graphs/data/bearRelHarami.txt";

const char* morningStarDirectory = "../graphs/data/morningStar.txt";
const char* eveningStarDirectory = "../graphs/data/eveningStar.txt";
const char* hammerDirectory = "../graphs/data/hammer.txt";
const char* llDojiDirectory = "../graphs/data/llDoji.txt";
const char* dfDojiDirectory = "../graphs/data/dfDoji.txt";
const char* gsDojiDirectory = "../graphs/data/gsDoji.txt";

const char* disagreementDirectory = "../graphs/data/disagreement.txt";
const char* consDisagreementDirectory = "../graphs/data/consDisagreement.txt";

const char* MACDDirectory = "../graphs/data/MACD.txt";
const char* sigMACDDirectory = "../graphs/data/sigMACD.txt";
const char* MACDbuySigDirectory = "../graphs/data/MACDbuySig.txt";
const char* MACDsellSigDirectory = "../graphs/data/MACDsellSig.txt";

const char* bollMiddleDirectory = "../graphs/data/bollMiddle.txt";
const char* bollUpperDirectory = "../graphs/data/bollUpper.txt";
const char* bollLowerDirectory = "../graphs/data/bollLower.txt";
const char* bollBuySigDirectory = "../graphs/data/bollBuySig.txt";
const char* bollSellSigDirectory = "../graphs/data/bollSellSig.txt";

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

    printDates(dayArray, DAYSRECORDED, datesDirectory);
    printFloatAttributes(dayArray, DAYSRECORDED, "close", closesDirectory);
    
    // Computing analysis tools:
    recordEngulfments(dayArray, DAYSRECORDED);
    recordStars(dayArray, DAYSRECORDED);
    recordHammers(dayArray, DAYSRECORDED, HAMMERSIZE);
    recordDojis(dayArray, DAYSRECORDED, DOJISIZE, DOJIEXTREMES);

    computeWilderRSI(dayArray, DAYSRECORDED, TIMEPERIOD);
    recordConsDisagreements(dayArray, DAYSRECORDED, TIMEPERIOD, disInterval, consecDis, disDist);

    recordMACD(dayArray, DAYSRECORDED, LOWERAVG, UPPERAVG);
    recordSigMACD(dayArray, DAYSRECORDED, UPPERAVG, SIGAVG);
    recordSigBuySell(dayArray, DAYSRECORDED, UPPERAVG+SIGAVG);

    recordBollingerBands(dayArray, DAYSRECORDED, BOLLPERIOD, BOLLSD);
    recordBollingerSignals(dayArray, DAYSRECORDED, BOLLPERIOD);

    // Printing results in /data subdirectory:

    printFloatAttributes(dayArray, DAYSRECORDED, "vol", volumesDirectory);  
    printFloatAttributes(dayArray, DAYSRECORDED, "RSI", RSIDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bullEngulf", bullEngulfDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bullRelEngulf", bullRelEngulfDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bearEngulf", bearEngulfDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bearRelEngulf", bearRelEngulfDirectory);

    printBoolAttributes(dayArray, DAYSRECORDED, "bullHarami", bullHaramiDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bullRelHarami", bullRelHaramiDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bearHarami", bearHaramiDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bearRelHarami", bearRelHaramiDirectory);

    printBoolAttributes(dayArray, DAYSRECORDED, "morningStar", morningStarDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "eveningStar", eveningStarDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "hammer", hammerDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "llDoji", llDojiDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "dfDoji", dfDojiDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "gsDoji", gsDojiDirectory);

    printBoolAttributes(dayArray, DAYSRECORDED, "disagreement", disagreementDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "consDisagreement", consDisagreementDirectory);

    printFloatAttributes(dayArray, DAYSRECORDED, "MACD", MACDDirectory);
    printFloatAttributes(dayArray, DAYSRECORDED, "sigMACD", sigMACDDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "MACDsellSig", MACDsellSigDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "MACDbuySig", MACDbuySigDirectory);

    printFloatAttributes(dayArray, DAYSRECORDED, "bollMiddle", bollMiddleDirectory);
    printFloatAttributes(dayArray, DAYSRECORDED, "bollUpper", bollUpperDirectory);
    printFloatAttributes(dayArray, DAYSRECORDED, "bollLower", bollLowerDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bollBuySig", bollBuySigDirectory);
    printBoolAttributes(dayArray, DAYSRECORDED, "bollSellSig", bollSellSigDirectory);


    return 0;
}