
#include "BackTester.cpp"

#include <string>
#include <stdio.h>


//*** Parameters: Bars, BackTester and Indicators (Divergence) required for generating signals: ***//

void 
basicDivStrategy(Bars* barsRef, BackTester* bt, 
    Indicators::Divergence* DivInd, Indicators::LocalMax* LocMax, Indicators::LocalMin* LocMin, Indicators::BollingerBands* BB,
    int maxBarsBack = 14)
{
    //*** INITIALIZING STRATEGY INDICATORS ***//
    DivInd->computeIndicator();
    LocMax->computeIndicator();
    LocMin->computeIndicator();
    BB->computeIndicator();

    //*** BAR ITERATION ***//
    for (int i = maxBarsBack; i < barsRef->getnumBars(); i++ )
    {
        //*** STRATEGY ENTRY CONDITION(S) ***//

            //** DENIED DIVERGENCE **//

        int barsBack;
        int tradeNo;

        // Find a local maximum ON PREVIOUS DAY: 
        if (LocMax->getIndicatorBar(i-1)->isPresent())       
        {
            // Find previous maximum:
            barsBack = 1;
            while (barsBack < maxBarsBack && !LocMax->getIndicatorBar(i-1-barsBack)->isPresent())
            {
                barsBack++;
            }

            // If
            // 1) current maximum is a NEW maximum, 
            // 2) current maximum has NO divergence,
            // 3) previous maximum has divergence 
            // then we have found a DENIED divergence,
            //  hence open trade on CURRENT DAY:

            if (barsRef->getBar(i-1)->getclose() > barsRef->getBar(i-1-barsBack)->getclose()
             && !DivInd->getIndicatorBar(i-1)->isPresent())
            {
                bt->openTrade(DivInd->getIndicatorBar(i-1-barsBack)->m, i, "Denied divergence on new local maximum");
            }
        }

        // Repeat on local minimums:
        if (LocMin->getIndicatorBar(i-1)->isPresent())
        {
            // Find previous minimum:
            barsBack = 1;
            while (barsBack < maxBarsBack && !LocMin->getIndicatorBar(i-1-barsBack)->isPresent())
            {
                barsBack++;
            }

            // Verify if we are on NEW minimum with NO divergence:
            if (barsRef->getBar(i-1)->getclose() < barsRef->getBar(i-1-barsBack)->getclose()
             && !DivInd->getIndicatorBar(i-1)->isPresent())
            {
                bt->openTrade(DivInd->getIndicatorBar(i-1-barsBack)->m, i, "Denied divergence on new local minimum");
            }
        }


        //*** STRATEGY EXIT CONDITION(S) ***//

        //*** Opposite (not necessarily denied) Divergence, on PREVIOUS bar ***/

        bt->closeTrades((-1) * DivInd->getIndicatorBar(i-1)->m, i, 
            "Opposite (not necessarily denied) divergence on previous bar");

        //*** Crossing bollinger bands ***/
            // Close any long POSITIVE trades when upper bollinger bands crossed
        if (barsRef->getBar(i)->getclose() > BB->getIndicatorBar(i)->bollUpper
         && barsRef->getBar(i-1)->getclose() < BB->getIndicatorBar(i-1)->bollUpper)
        {
            bt->closeTrades(1, i, "Crossed upper Bollinger Bands", true, false);
        }
            // Close any short POSITIVE trades when lower bollinger bands crossed
        if (barsRef->getBar(i)->getclose() < BB->getIndicatorBar(i)->bollLower
         && barsRef->getBar(i-1)->getclose() > BB->getIndicatorBar(i-1)->bollLower)
        {
            bt->closeTrades(-1, i, "Crossed lower Bollinger Bands", true, false);
        }


        //*** UPDATE TRADES (TAKE PROFIT & STOP LOSS) ***//

        bt->updateTrades(i);
    }

    //*** PRINT RESULTS ***//

    bt->printResults();
}



///********* TESTING *********///

int main()
{
    Bars* Bars = new ::Bars(9, 100000, "../data/dax futures hourly.txt", "2022-04-11 11:00:00", "2023-03-30 17:00:00");
     basicDivStrategy(Bars, new BackTester(Bars, 5.0, 4.0), 
        initDivergences(Bars), new Indicators::LocalMax(Bars), new Indicators::LocalMin(Bars), new Indicators::BollingerBands(Bars));
    return 0;
}
