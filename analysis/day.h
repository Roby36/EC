

#ifndef __DAY_H
#define __DAY_H


/************************************** DAY ANALYSIS TOOLS **********************************************/

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

    // Moving averages indicators:
    float MACD;
    float sigMACD;
    bool MACDbuySig;
    bool MACDsellSig;

    // Japanese candlesticks indicators:
    bool bullEngulf;
    bool bullRelEngulf;
    bool bearEngulf;
    bool bearRelEngulf;

    bool bullHarami;
    bool bullRelHarami;
    bool bearHarami;
    bool bearRelHarami; 

    bool morningStar;
    bool eveningStar;

    bool hammer;

    bool llDoji;
    bool dfDoji;
    bool gsDoji;

    //Bollinger bands:
    float bollMiddle;
    float bollUpper;
    float bollLower;
    bool bollBuySig;
    bool bollSellSig;

    // "Consecutive disagreements" indicators:
    bool disagreement;
    bool consDisagreement;

} day_t;


/************************ TECHNICAL ANALYSIS TOOLS ***************************/

void computeWilderRSI(day_t *dayArray[], const int daysRecorded, const int timePeriod);

void recordEngulfments(day_t *dayArray[], const int daysRecorded);

void recordStars(day_t *dayArray[], const int daysRecorded);

void recordHammers(day_t *dayArray[], const int daysRecorded, const int hammerSize);

void recordDojis(day_t *dayArray[], const int daysRecorded, const int dojiSize, const int dojiExtremes);

void recordMACD(day_t *dayArray[], const int daysRecorded, const int lower, const int upper);

void recordSigMACD(day_t *dayArray[], const int daysRecorded, const int MACDupper, const int avgPeriod);

void recordSigBuySell(day_t *dayArray[], const int daysRecorded, const int totPeriod);

void recordBollingerBands(day_t *dayArray[], const int daysRecorded, const int timePeriod, float standardDeviations);

void recordBollingerSignals(day_t *dayArray[], const int daysRecorded, const int timePeriod);


/**************** ALGORITHMS / COMPOSITE TOOLS *********************/

void recordConsDisagreements(day_t *dayArray[], const int daysRecorded, const int timePeriod, const int disInterval, const int consecDis, const int disDist);














#endif // __DAY_H