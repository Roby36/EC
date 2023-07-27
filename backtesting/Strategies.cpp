
#include "BackTester.cpp"
#include "../analysis/IndicatorSet.cpp"

/*************************************/
//*  Parameters:                     */
/*    - Bars                         */                 
/*    - BackTester                   */
/*    - Initialized & computed       */
/*       Indicators                  */
/*************************************/

const int bpd = 9;
const int maxBars = 2*bpd;

void 
testStrategy(Bars* barsRef, BackTester* bt, IndicatorSet* Indicators, int maxBarsBack = 14)
{
    /*************************************/
    //*** SET TAKE PROFIT & STOP LOSS ***//
    /*************************************/
    double takeProfit, stopLoss;
    #ifdef HOURLY
        takeProfit = 2.5; stopLoss = 2.5;
    #endif 
    #ifdef DAILY
        takeProfit = 5.0; stopLoss = 4.0;
    #endif 
    #ifdef SE
        takeProfit = 1.5; stopLoss = 1.5;
    #endif 
    
    /*************************************/
    /******** START BAR ITERATION ********/
    /*************************************/
    for (int i = maxBarsBack; i < barsRef->getnumBars(); i++ )
    {
        /*************************************/
        /**** STRATEGY ENTRY CONDITION(S) ****/
        /*************************************/

        #if defined(S1) || defined(S1P)  || defined(SE) || defined(S4)

            //***** DENIED DIVERGENCE *****//

            int barsBack;
            int tradeNo;
            // Find a local maximum ON PREVIOUS DAY: 
            if (Indicators->LocalMax->getIndicatorBar(i-1)->isPresent())       
            {
                // Find previous maximum:
                barsBack = 1;
                while (barsBack < maxBarsBack && !Indicators->LocalMax->getIndicatorBar(i-1-barsBack)->isPresent()) {
                    barsBack++;
                }
                // If
                // 1) current maximum is a NEW maximum, 
                // 2) current maximum has NO divergence,
                // 3) previous maximum has divergence 
                // then we have found a DENIED divergence,
                //  hence open trade on CURRENT DAY:
                if (barsRef->getBar(i-1)->close() > barsRef->getBar(i-1-barsBack)->close()
                && !Indicators->Divergence->getIndicatorBar(i-1)->isPresent()) 
                {
                    bt->openTrade(Indicators->Divergence->getIndicatorBar(i-1-barsBack)->m, i, "Denied divergence on new local maximum");
                }
            }
            // Repeat on local minimums:
            if (Indicators->LocalMin->getIndicatorBar(i-1)->isPresent())
            {
                // Find previous minimum:
                barsBack = 1;
                while (barsBack < maxBarsBack && !Indicators->LocalMin->getIndicatorBar(i-1-barsBack)->isPresent()) {
                    barsBack++;
                }
                // Verify if we are on NEW minimum with NO divergence:
                if (barsRef->getBar(i-1)->close() < barsRef->getBar(i-1-barsBack)->close()
                && !Indicators->Divergence->getIndicatorBar(i-1)->isPresent()) {
                    bt->openTrade(Indicators->Divergence->getIndicatorBar(i-1-barsBack)->m, i, "Denied divergence on new local minimum");
                }
            }
        #endif  // defined(S1) || defined(S1P) || defined(SE) || defined(S4)

        #if defined(S2) || defined(S2P)

            //** DOUBLE DIVERGENCE **//

            // Find double divergence on PREVIOUS day
            if (Indicators->Divergence->getIndicatorBar(i-1)->divPoints == 3) {
                bt->openTrade(Indicators->Divergence->getIndicatorBar(i-1)->m, i, "Double divergence");
            }
        #endif  // defined(S2) || defined(S2P)


        /*************************************/
        /**** STRATEGY EXIT CONDITION(S) ****/
        /*************************************/
        #ifndef SE

        #if defined(S1) || defined(S1P) || defined(S2) || defined(S2P) || defined(S4)
            //** OPPOSITE DIVERGENCE **//
            bt->closeTrades((-1) * Indicators->Divergence->getIndicatorBar(i-1)->m, i, 
                "Opposite (not necessarily denied) divergence on previous bar");
        #endif // defined(S1) || defined(S1P) || defined(S2) || defined(S2P) || defined(S4)

            //** CROSSING BOLLINGER BANDS **//

            bool takeProfits;
        #if defined(S1) || defined(S2) || defined(S4)
            takeProfits = true;
        #endif
        #if defined(S1P) || defined(S2P)
            takeProfits = false;
        #endif

            // Close any LONG trades when upper bollinger bands crossed from ABOVE
            if (Indicators->BollingerBands->getIndicatorBar(i)->crossUpperDown) 
            {
                bt->closeTrades(1, i, "Crossed upper Bollinger Bands from above", takeProfits, false);
            }
            // Close any SHORT trades when lower bollinger bands crossed from BELOW
            if (Indicators->BollingerBands->getIndicatorBar(i)->crossLowerUp) 
            {
                bt->closeTrades(-1, i, "Crossed lower Bollinger Bands from below", takeProfits, false);
            }

            /**** Additional S4 conditions ****/
        #ifdef S4
            // Close all EXPIRED, NEGATIVE long & short trades
            bt->closeTrades(1, i, "Negative after expiration time", false, true, 18);
            bt->closeTrades(-1, i, "Negative after expiration time", false, true, 18);
        #endif // S4

        
        #endif // SE
        /**********************************************/
        /**** UPDATE TRADES (END OF BAR ITERATION) ****/
        /**********************************************/

        bt->updateTrades(i, takeProfit, stopLoss);
    }
}



///********* TESTING *********///

int main(const int argc, const char* argv[])
{
    //*** VALIDATING ARGUMENTS ***//
    if (argc != 2) {
        fprintf(stderr, "Usage: %s inputFileDirectory\n", argv[0]);
        exit(2);
    }

    //*** DETERMINE OUTPUT FILE EXTENSIONS ***//
    string outPath = string(argv[1]);

    #ifdef S1
    outPath += ".S1";
    #endif
    #ifdef S2
    outPath += ".S2";
    #endif
    #ifdef S1P
    outPath += ".S1P";
    #endif
    #ifdef S2P
    outPath += ".S2P";
    #endif
    #ifdef SE
    outPath += ".SE";
    #endif
    #ifdef S4
    outPath += ".S4";
    #endif

    #ifdef HOURLY
    outPath += "hourly.txt";
    #endif
    #ifdef DAILY
    outPath += "daily.txt";
    #endif

    //*** INITIALIZING BARS, INDICATORS, BACKTESTER ***//
    ::Bars* Bars = new ::Bars(argv[1]); 
    ::IndicatorSet* IndicatorSet = new ::IndicatorSet(Bars);
    ::BackTester* bt = new BackTester(Bars, 3, outPath.c_str(), outPath.c_str());

    //*** RUNNING STRATEGY ***//
    testStrategy(Bars, bt, IndicatorSet);

    //*** PRINT RESULTS & INDICATOR DATA ***//
    bt->printResults();

    #ifdef ILOG
    Indicators->printIndicators();
    #endif // ILOG
    
    //*** CLEANING UP ***//
    delete(bt); 
    delete(IndicatorSet);
    delete(Bars); 

    return 0;
}

