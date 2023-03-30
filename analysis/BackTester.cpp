
#include "BackTester.h"
#include "Indicators.cpp"

#include <string>
#include <stdio.h>

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

    int close(int exitPos)
    {
        if (this->isActive())
        {
            this->exitPos = exitPos;
            return this->tradeNo;
        }
        return -1;
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


int BackTester::openTrade(int direction, int entryPos, string reason, float units)
{
    // Verify validity of direction:
    if (direction != -1 && direction != 1) 
    {
        /* this->logTrade("Could not open trade on " + string(this->barsRef->getBar(entryPos)->getdateTime()) + " ", 
            this->currTradeNo, "invalid direction provided"); */
        return -1; 
    }
    // First attempt to close any trades in opposite direction:
    for (int i = 0; i < this->currTradeNo; i++)
    {
        if (this->execTrades[i]->getdir() != direction)
        {
            this->closeTrade(this->execTrades[i], entryPos, "Opened trade in opposite direction");
        }
    }
    // Verify that not too many trades are already open:
    if (this->openTrades == this->maxTrades) 
    {
        this->logTrade("Could not open trade on " + string(this->barsRef->getBar(entryPos)->getdateTime()) + " ", 
            this->currTradeNo, "maximum number of trades reached");
        return -1; 
    }
    // Open new trade
    Trade* newTrade = new Trade(this->barsRef, this->currTradeNo, direction, entryPos, units);
    this->execTrades[this->currTradeNo] = newTrade;
    this->logTrade("Opened ", this->currTradeNo, reason);
    this->openTrades++;
    return this->currTradeNo++;
}


int BackTester::closeTrade(Trade* trade, int exitPos, string reason)
{
    int tradeNo;
    if ((tradeNo = trade->close(exitPos)) != -1)
    {
        this->logTrade("Closed", tradeNo, reason);
        this->openTrades--;
    }
    return tradeNo;
}


void BackTester::closeTrades(int dir, int exitPos, string reason, bool takeProfits, bool stopLosses)
{
    Trade* currTrade;
    // Iterate through all active and inactive trades 
    for (int i = 0; i < this->currTradeNo; i++)
    {
        currTrade = this->execTrades[i];
        // If current Trade is in the desired direction attempt to close it at current position:
        if (currTrade->getdir() == dir)
        {
            if (takeProfits) // If taking profits, close ONLY positive trades
            {
                if (currTrade->currBal(exitPos) > 0)
                {
                    this->closeTrade(currTrade, exitPos, "Taking Profit: " + reason);
                }
            }
            else if (stopLosses)
            {
                if (currTrade->currBal(exitPos) < 0)
                {
                    this->closeTrade(currTrade, exitPos, "Stopping Loss: " + reason);
                }
            }
            else
            {
                this->closeTrade(currTrade,exitPos, reason);
            }
        }
    }
}


void BackTester::updateTrades(int currPos)
{
    int tradeNo;
    int currPl = 0;
    for (int i = 0; i < this->currTradeNo; i++)
    {
        // First update total balance with current active or not active trade:
        currPl += this->execTrades[i]->currBal(currPos);
        // Then check if trade needs to be closed
        if (this->takeProfThresh > 0.1f &&  100.0f * this->execTrades[i]->currBal(currPos) / this->barsRef->getBar(currPos)->getclose() > this->takeProfThresh
        ||  this->stopLossThresh > 0.1f && -100.0f * this->execTrades[i]->currBal(currPos) / this->barsRef->getBar(currPos)->getclose() > this->stopLossThresh)
        {
            this->closeTrade(this->execTrades[i], currPos, "reached stop loss / take profit point");
        } 
    }
    this->pl = currPl;
    this->plArray[currPos] = this->pl;
}


void BackTester::logTrade(string action, int tradeNo, string reason)
{
    this->tradeLog += action + " " + to_string(tradeNo) + ": " + reason + "\n";
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

    fp = fopen(this->tradeLogPath, "a");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening tradeLogPath for appending\n");
        return;
    }
    fprintf(fp, "%s", this->tradeLog.c_str());
    fclose(fp);
}

