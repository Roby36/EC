
#include "BackTester.cpp"

#include <string>
#include <stdio.h>


//*** Parameters: Bars, BackTester and INITIALIZED & COMPUTED Indicators (Divergence) required for generating signals: ***//

void 
basicDivStrategy(Bars* barsRef, BackTester* bt, 
    Indicators::Divergence* DivInd, Indicators::LocalMax* LocMax, Indicators::LocalMin* LocMin, Indicators::BollingerBands* BB,
    int maxBarsBack = 14)
{
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
            // Close any LONG POSITIVE trades when upper bollinger bands crossed from ABOVE
        if (BB->getIndicatorBar(i)->crossUpperDown)
        {
            bt->closeTrades(1, i, "Crossed upper Bollinger Bands from above", true, false);
        }
            // Close any SHORT POSITIVE trades when lower bollinger bands crossed from BELOW
        if (BB->getIndicatorBar(i)->crossLowerUp)
        {
            bt->closeTrades(-1, i, "Crossed lower Bollinger Bands from below", true, false);
        }


        //*** UPDATE TRADES (TAKE PROFIT & STOP LOSS) ***//

        bt->updateTrades(i);
    }

    //*** PRINT RESULTS ***//

    bt->printResults();
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

    //*** INITIALIZING BARS & STRATEGY'S INDICATORS ***//
    ::Bars* Bars = new ::Bars(9, 10000, argv[1]);
    Indicators::RSI* RSI = new Indicators::RSI(Bars);
    Indicators::LocalMax* LocMax = new Indicators::LocalMax(Bars);
    Indicators::LocalMin* LocMin = new Indicators::LocalMin(Bars);
    Indicators::Divergence* DivInd = new Indicators::Divergence(Bars,RSI,LocMax,LocMin);
    Indicators::BollingerBands* BB = new Indicators::BollingerBands(Bars);

    //*** COMPUTING INDICATORS ***//
    RSI->computeIndicator();
    LocMax->computeIndicator();
    LocMin->computeIndicator();
    DivInd->computeIndicator();
    BB->computeIndicator();

    //*** INITIALIZING BACKTESTER ***//
    ::BackTester* bt = new BackTester(Bars, 5.0, 4.0);

    //*** RUNNING STRATEGY ***//
    basicDivStrategy(Bars, bt, DivInd, LocMax, LocMin, BB);
    
    //*** CLEANING UP ***//
    bt->Delete(); 
    DivInd->Delete(); delete(DivInd);
    BB->Delete();     delete(BB);
    LocMax->Delete(); delete(LocMax);
    LocMin->Delete(); delete(LocMin);
    RSI->Delete();    delete(RSI);
    Bars->Delete(); 

    return 0;
}
