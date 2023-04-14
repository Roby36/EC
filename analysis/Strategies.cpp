
#include "BackTester.cpp"

#include <string>
#include <stdio.h>


//*** Parameters: Bars, BackTester and INITIALIZED & COMPUTED Indicators (Divergence) required for generating signals: ***//
void 
testStrategy(Bars* barsRef, BackTester* bt, Indicators* Indicators, int maxBarsBack = 14)
{
    //*** SET TAKE PROFIT & STOP LOSS ***//
    float takeProfit, stopLoss;

    #if defined(S1) || defined(S2)
        takeProfit = 5.0f; stopLoss = 4.0f;
    #endif  // defined(S1) || defined(S2)

    //*** BAR ITERATION ***//
    for (int i = maxBarsBack; i < barsRef->getnumBars(); i++ )
    {
        //*** STRATEGY ENTRY CONDITION(S) ***//

        #if defined(S1)
            //** DENIED DIVERGENCE **//

            int barsBack;
            int tradeNo;
            // Find a local maximum ON PREVIOUS DAY: 
            if (Indicators->LocalMax->getIndicatorBar(i-1)->isPresent())       
            {
                // Find previous maximum:
                barsBack = 1;
                while (barsBack < maxBarsBack && !Indicators->LocalMax->getIndicatorBar(i-1-barsBack)->isPresent())
                {
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
                while (barsBack < maxBarsBack && !Indicators->LocalMin->getIndicatorBar(i-1-barsBack)->isPresent())
                {
                    barsBack++;
                }
                // Verify if we are on NEW minimum with NO divergence:
                if (barsRef->getBar(i-1)->close() < barsRef->getBar(i-1-barsBack)->close()
                && !Indicators->Divergence->getIndicatorBar(i-1)->isPresent())
                {
                    bt->openTrade(Indicators->Divergence->getIndicatorBar(i-1-barsBack)->m, i, "Denied divergence on new local minimum");
                }
            }
        #endif  // defined(S1)

        #if defined(S2)

            //** DOUBLE DIVERGENCE **//
            // Find double divergence on PREVIOUS day
            if (Indicators->Divergence->getIndicatorBar(i-1)->divPoints == 3)
            {
                bt->openTrade(Indicators->Divergence->getIndicatorBar(i-1)->m, i, "Double divergence");
            }
        #endif  // defined(S2)


        //*** STRATEGY EXIT CONDITION(S) ***//

        #if defined(S1) || defined(S2)
            //*** Opposite (not necessarily denied) Divergence, on PREVIOUS bar ***/

            bt->closeTrades((-1) * Indicators->Divergence->getIndicatorBar(i-1)->m, i, 
                "Opposite (not necessarily denied) divergence on previous bar");

            //*** Crossing bollinger bands ***/
                // Close any LONG POSITIVE trades when upper bollinger bands crossed from ABOVE
            if (Indicators->BollingerBands->getIndicatorBar(i)->crossUpperDown)
            {
                bt->closeTrades(1, i, "Crossed upper Bollinger Bands from above", true, false);
            }
                // Close any SHORT POSITIVE trades when lower bollinger bands crossed from BELOW
            if (Indicators->BollingerBands->getIndicatorBar(i)->crossLowerUp)
            {
                bt->closeTrades(-1, i, "Crossed lower Bollinger Bands from below", true, false);
            }
        #endif  // defined(S1) || defined(S2)

        //*** UPDATE TRADES (TAKE PROFIT & STOP LOSS) ***//
        bt->updateTrades(i, takeProfit, stopLoss);
    }
}



///********* TESTING *********///

int main(const int argc, const char* argv[])
{
    //*** VALIDATING ARGUMENTS ***//
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s inputFileDirectory\n", argv[0]);
        exit(2);
    }

    //*** DETERMINE OUTPUT FILE EXTENSIONS ***//
    string outPath = string(argv[1]);
    #ifdef S1
    outPath += ".S1.txt";
    #endif
    #ifdef S2
    outPath += ".S2.txt";
    #endif

    //*** INITIALIZING BARS, INDICATORS, BACKTESTER ***//
    ::Bars* Bars = new ::Bars(argv[1]); 
    ::Indicators* Indicators = new ::Indicators(Bars);
    ::BackTester* bt = new BackTester(Bars, 3, outPath.c_str(), outPath.c_str());

    //*** RUNNING STRATEGY ***//
    testStrategy(Bars, bt, Indicators);

    //*** PRINT RESULTS ***//
    bt->printResults();
    
    //*** CLEANING UP ***//
    bt->Delete(); 
    Indicators->Delete();
    Bars->Delete(); 

    return 0;
}

