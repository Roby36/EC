
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


BackTester::BackTester(Bars* barsRef, 
    float takeProfThresh, float stopLossThresh, int maxTrades, char* reportPath, char* logPath)
{
    this->barsRef = barsRef;
    this->execTrades = new Trade*[barsRef->getnumBars()];
    this->plArray = new float[barsRef->getnumBars()];
    this->takeProfThresh = takeProfThresh;
    this->stopLossThresh = stopLossThresh;
    this->maxTrades = maxTrades;
    this->reportPath = reportPath;
    this->logPath = logPath;
}


void BackTester::Delete()
{
    for(int i = 0; i < this->currTradeNo; i++)
    {
        delete(this->execTrades[i]);
    }
    delete(this->execTrades);
    delete(this->plArray);
    delete(this);
}


int BackTester::openTrade(int direction, int entryPos, string reason, float units)
{
    // Verify validity of direction:
    if (direction != -1 && direction != 1) 
    {
        return -1; 
    }
    // First attempt to close any trades in opposite direction:
    for (int i = 0; i < this->currTradeNo; i++)
    {
        if (this->execTrades[i]->getdir() != direction)
        {
            this->closeTrade(i, entryPos, "Opened trade in opposite direction");
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
    this->openTrades++;
    this->logTrade("Opened ", this->currTradeNo, reason + ". Currently open trades: " + to_string(this->openTrades));
    return this->currTradeNo++;
}


bool BackTester::closeTrade(int tradeNo, int exitPos, string reason)
{
    if (tradeNo < 0 || tradeNo >= currTradeNo)
    {
        this->logTrade("Could not close", tradeNo, "invalid trade number");
    }

    Trade* trade = this->execTrades[tradeNo];
    if (trade->close(exitPos))
    {
        this->logTrade("Closed", tradeNo, reason /* + ". Current P&L: " + to_string(this->pl) + ";" */);
        this->openTrades--;
        return true;
    }
    return false;
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
                    this->closeTrade(i, exitPos, reason + " (take profit)");
                }
            }
            else if (stopLosses)
            {
                if (currTrade->currBal(exitPos) < 0)
                {
                    this->closeTrade(i, exitPos, reason + " (stop loss)");
                }
            }
            else
            {
                this->closeTrade(i,exitPos, reason);
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
            this->closeTrade(i, currPos, "reached stop loss / take profit point");
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
    // First compute positive and negative trades, at LAST BAR:
    int posTrades = 0; float posBalance = 0.0f;
    int negTrades = 0; float negBalance = 0.0f;
    float currBal;
    for (int i = 0; i < this->currTradeNo; i++)
    {
        currBal = this->execTrades[i]->currBal(this->barsRef->getnumBars()-1);
        if (currBal > 0)
        {
            posTrades++;
            posBalance += currBal;
        }
        else
        {
            negTrades++;
            negBalance += currBal;
        }
    }

    FILE* fp;

    // Print strategy report:
    if (this->reportPath != NULL)
    {
        fp = fopen(this->reportPath, "w");
        if (fp != NULL) { fclose(fp); }
        fp = fopen(this->reportPath, "a");
        if (fp == NULL)
        {
            fprintf(stderr, "Error opening reportPath for appending\n");
            return;
        }
    }
    else
    {
        fp = stdout;
    }
    fprintf(fp, "\n STRATEGY REPORT:\nTotal positive trades: %d (%f points); Total negative trades: %d (%f points);\n Net loss/profit: %f\n\n", 
        posTrades, posBalance, negTrades, negBalance, this->pl);
    for (int i = 0; i < this->currTradeNo; i++)
    {
        fprintf(fp, "%s", this->execTrades[i]->print().c_str());
    }
    if (this->reportPath != NULL) { fclose(fp); }

    // Print log:
    if (this->logPath != NULL)
    {
        fp = fopen(this->logPath, "w");
        if (fp != NULL) { fclose(fp); }
        fp = fopen(this->logPath, "a");
        if (fp == NULL)
        {
            fprintf(stderr, "Error opening logPath for appending\n");
            return;
        }
    }
    else
    {
        fp = stdout;
    }
    fprintf(fp, "\n TRADE LOG:\n%s\n END OF LOG\n\n", this->tradeLog.c_str());
    if (this->logPath != NULL) { fclose(fp); }

    #ifdef PL
    // Finally print P&L data to bar's data output file:
    fp = fopen((this->barsRef->getoutputDir() + "PL" + this->barsRef->getoutputExt()).c_str(), "w");
    if (fp != NULL) { fclose(fp); }
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

    #endif //NOPL
}

