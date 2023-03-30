
#include "BackTester.h"
#include "Bars.h"
#include "Indicators.cpp"

#include <string>

class BackTester::Trade
{
    Bars* barsRef; 

    const float multFactor = 1.0;
    int tradeNo;
    int entryPos;
    int exitPos;
    int direction;

    float units = 1.0;

    public:

    Trade(Bars* barsRef,  int currTradeNo, int direction, int entryPos, int units)
    {
        this->barsRef = barsRef; 
        this->direction = direction;
        this->entryPos = entryPos;
        this->exitPos = entryPos;
        this->units = units;
        this->tradeNo = currTradeNo;
    }

    bool close(int exitPos)
    {
        if (this->isActive())
        {
            this->exitPos = exitPos;
            return true;
        }
        return false;
    }

    float currBal(int currPos)
    {
        if (this->isActive())
        {
            return
            (this->units)*(this->multFactor)*(this->direction)*
            (this->barsRef->getBar(currPos)->getclose() - this->barsRef->getBar(this->entryPos)->getclose());
        }
        else
        {
            return
            (this->units)*(this->multFactor)*(this->direction)*
            (this->barsRef->getBar(this->exitPos)->getclose() - this->barsRef->getBar(this->entryPos)->getclose());
        }
    }

    bool isActive()
    {
        return this->exitPos == this->entryPos;
    }

    string print()
    {
        string dir;
        if (this->direction == 1) { dir = "long"; }
        else { dir = "short"; }

        return(
            " Trade no: " + to_string(this->tradeNo) + ";"
            + " Direction: " + dir + ";"
            + " Entry date: " + string(this->barsRef->getBar(entryPos)->getdateTime()) + ";"
            + " Entry closing price: " + to_string(this->barsRef->getBar(entryPos)->getclose()) + ";"
            + " Exit date: " + string(this->barsRef->getBar(exitPos)->getdateTime()) + ";"
            + " Exit closing price: " + to_string(this->barsRef->getBar(exitPos)->getclose()) + ";"
            + " Loss/profit: " + to_string(this->currBal(exitPos)) + ";"
            + "\n"
        );
    }

    /*** GETTERS ***/
    int getdir() const { return this->direction; }    

};

BackTester::BackTester(Bars* barsRef, float takeProfThresh, float stopLossThresh)
{
    this->barsRef = barsRef;
    this->execTrades = new Trade*[barsRef->getnumBars()];
    this->plArray = new float[barsRef->getnumBars()];
    this->takeProfThresh = takeProfThresh;
    this->stopLossThresh = stopLossThresh;
}

bool BackTester::openTrade(int direction, int entryPos, float units)
{
    // Verify validity of direction:
    if (direction != -1 && direction != 1) { return false; }
    // First attempt to close any trades in opposite direction:
    for (int i = 0; i < this->currTradeNo; i++)
    {
        if (this->execTrades[i]->getdir() != direction)
        {
            this->closeTrade(this->execTrades[i], entryPos);
        }
    }
    // Verify that not too many trades are already open:
    if (this->openTrades == this->maxTrades) { return false; }
    // Open new trade
    Trade* newTrade = new Trade(/* this,*/ this->barsRef, this->currTradeNo, direction, entryPos, units);
    this->execTrades[this->currTradeNo++] = newTrade;
    this->openTrades++;
    return true;
}

void BackTester::closeTrade(Trade* trade, int exitPos)
{
    if (trade->close(exitPos))
    {
        this->openTrades--;
    }
}

void BackTester::updateTrades(int currPos)
{
    for (int i = 0; i < this->currTradeNo; i++)
    {
        // First update total balance with current active or not active trade:
        this->pl = this->execTrades[i]->currBal(currPos);
        // Then check if trade needs to be closed
        if (this->takeProfThresh > 0.1f &&  100.0f * this->execTrades[i]->currBal(currPos) / this->barsRef->getBar(currPos)->getclose() > this->takeProfThresh
        ||  this->stopLossThresh > 0.1f && -100.0f * this->execTrades[i]->currBal(currPos) / this->barsRef->getBar(currPos)->getclose() > this->stopLossThresh)
        {
            this->closeTrade(this->execTrades[i],currPos);
        } 
    }
    this->plArray[currPos] = this->pl;
}

void BackTester::printResults()
{
    FILE* fp = fopen(this->outputPath, "a");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening outputPath for appending\n");
        return;
    }
    fprintf(fp, "Net loss/profit: %f (%d total trades)\n", this->pl, this->currTradeNo);
    for (int i = 0; i < this->currTradeNo; i++)
    {
        fprintf(fp, "%s", this->execTrades[i]->print().c_str());
    }
    fclose(fp);

    fp = fopen((this->barsRef->getoutputDir() + "PL" + this->barsRef->getoutputExt()).c_str(), "a");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening PL output file for appending\n");
        return;
    }
    for (int i = 0; i < this->barsRef->getnumBars(); i++)
    {
        fprintf(fp, "%s", (to_string(this->plArray[i]) + "\n").c_str());
    }
    fclose(fp);
}




//****** TEST STRATEGY ******//

// Parameters: Bars, BackTester and Indicators (Divergence) required for generating signals:
void basicDivStrategy(Bars* barsRef, BackTester* bt, Indicators::Divergence* DivInd)
{
    //Compute (divergence) indicator(s):
    DivInd->computeIndicator();

    // Iterate through bars:
    for (int i = 1; i < barsRef->getnumBars(); i++ )
    {
        //*** STRATEGY ENTRY CONDITION(S) ***//
            // Entry based on PREVIOUS day's divergence:
        bt->openTrade(DivInd->getIndicatorBar(i-1)->m, i);
        
        // After iteration, update all trades (EXIT CONDITION INCLUDED)
        bt->updateTrades(i);
    }

    // Finally, print results
    bt->printResults();
}


int main()
{
    Bars* Bars = new ::Bars(24, 2800, "../data/DAX Historical Data.csv", "02/24/2022", "02/24/2023");
    basicDivStrategy(Bars, new BackTester(Bars, 4.0, 4.0), 
        initDivergences(Bars));
    return 0;
}
