
#include "BackTester.h"

BackTester::BackTester(Bars* barsRef, const int maxTrades, const char* reportPath, const char* logPath)
    : reportPath(reportPath), logPath(logPath)
{
    this->barsRef = barsRef;
    this->execTrades = new Trade*[barsRef->getnumBars()];
    this->plArray = new double[barsRef->getnumBars()];
    this->maxTrades = maxTrades;
}

void BackTester::Delete()
{
    for(int i = 0; i < this->currTradeNo; i++) {
        delete(this->execTrades[i]);
    }
    delete(this->execTrades);
    delete(this->plArray);
    delete(this);
}

int BackTester::openTrade(int direction, int entryPos, string reason, double units)
{
    // Verify validity of direction:
    if (direction != -1 && direction != 1) {
        return -1; 
    }
    // First attempt to close any trades in opposite direction:
    for (int i = 0; i < this->currTradeNo; i++) {
        if (this->execTrades[i]->getdir() != direction) {
            this->closeTrade(i, entryPos, "Opened trade in opposite direction");
        }
    }
    // Verify that not too many trades are already open:
    if (this->openTrades == this->maxTrades) {
        this->logTrade("Could not open trade on " + string(this->barsRef->getBar(entryPos)->date_time_str) + " ", 
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
    if (tradeNo < 0 || tradeNo >= currTradeNo) {
        this->logTrade("Could not close", tradeNo, "invalid trade number");
    }

    Trade* trade = this->execTrades[tradeNo];
    if (trade->close(exitPos)) {
        this->logTrade("Closed", tradeNo, reason /* + ". Current P&L: " + to_string(this->pl) + ";" */);
        this->openTrades--;
        return true;
    }
    return false;
}

void BackTester::closeTrades(int dir, int exitPos, string reason, bool takeProfits, bool stopLosses, int barLim)
{
    Trade* currTrade;
    bool expired; // protects trade from closing until expiration 
    for (int i = 0; i < this->currTradeNo; i++) // Iterate through all active and inactive trades
    {
        currTrade = this->execTrades[i];
        // RESET EXPIRED to false for each trade!
        expired = (exitPos - currTrade->getEntryPos() > barLim);
        // If current Trade is in the desired direction and EXPIRED attempt to close it at current position:
        if (currTrade->getdir() == dir && expired)
        {
            if (takeProfits) // If taking profits, close ONLY positive trades
            {
                if (currTrade->currBal(exitPos) > 0) {
                    this->closeTrade(i, exitPos, reason + " (take profit)");
                }
            }
            else if (stopLosses) // If stopping losses, close ONLY negative trades
            {
                if (currTrade->currBal(exitPos) < 0) {
                    this->closeTrade(i, exitPos, reason + " (stop loss)");
                }
            }
            else
            {
                this->closeTrade(i, exitPos, reason);
            }
        }
    }
}

void BackTester::updateTrades(int currPos, double takeProfThresh, double stopLossThresh)
{
    int tradeNo;
    double currPl = 0.0;
    for (int i = 0; i < this->currTradeNo; i++) {
        // First update total balance with current active or not active trade:
        currPl += this->execTrades[i]->currBal(currPos);
        // Then check if trade needs to be closed
        if (takeProfThresh > 0.1f &&  100.0 * this->execTrades[i]->currBal(currPos) / this->barsRef->getBar(currPos)->close() > takeProfThresh
        ||  stopLossThresh > 0.1f && -100.0 * this->execTrades[i]->currBal(currPos) / this->barsRef->getBar(currPos)->close() > stopLossThresh)
        {
            this->closeTrade(i, currPos, "reached stop loss / take profit point");
        } 
    }
    this->pl = currPl;
    this->plArray[currPos] = this->pl;
}

void BackTester::logTrade(string action, int tradeNo, string reason) {
    this->tradeLog += action + " " + to_string(tradeNo) + ": " + reason + "\n";
}

void BackTester::printResults()
{
    // First compute positive and negative trades, at LAST BAR:
    int posTrades = 0; double posBalance = 0.0;
    int negTrades = 0; double negBalance = 0.0;
    double currBal;
    for (int i = 0; i < this->currTradeNo; i++) {
        currBal = this->execTrades[i]->currBal(this->barsRef->getnumBars()-1);
        if (currBal > 0) {
            posTrades++;
            posBalance += currBal;
        }
        else {
            negTrades++;
            negBalance += currBal;
        }
    }
    FILE* fp;
    // Print strategy report:
    if (this->reportPath != NULL) {
        fp = fopen(this->reportPath, "w");
        if (fp != NULL) { fclose(fp); }
        fp = fopen(this->reportPath, "a");
        if (fp == NULL) {
            fprintf(stderr, "Error opening reportPath for appending\n");
            return;
        }
    }
    else {
        fp = stdout;
    }
    fprintf(fp, "\n STRATEGY REPORT:\nTotal positive trades: %d (%f points); Total negative trades: %d (%f points);\n Net loss/profit: %f\n\n", 
        posTrades, posBalance, negTrades, negBalance, this->pl);
    for (int i = 0; i < this->currTradeNo; i++) {
        fprintf(fp, "%s", this->execTrades[i]->print().c_str());
    }
    if (this->reportPath != NULL) { fclose(fp); }

    // Print log:
    if (this->logPath != NULL) {
        /*
        fp = fopen(this->logPath, "w");
        if (fp != NULL) { fclose(fp); }
        */
        fp = fopen(this->logPath, "a");
        if (fp == NULL) {
            fprintf(stderr, "Error opening logPath for appending\n");
            return;
        }
    }
    else {
        fp = stdout;
    }
    fprintf(fp, "\n TRADE LOG:\n%s\n END OF LOG\n\n", this->tradeLog.c_str());
    if (this->logPath != NULL) { fclose(fp); }

    #ifdef PL
    // Finally print P&L data to bar's data output file:
    fp = fopen((this->barsRef->getoutputDir() + "PL" + this->barsRef->getoutputExt()).c_str(), "w");
    if (fp != NULL) { fclose(fp); }
    fp = fopen((this->barsRef->getoutputDir() + "PL" + this->barsRef->getoutputExt()).c_str(), "a");
    if (fp == NULL) {
        fprintf(stderr, "Error opening PL output file for appending\n");
        return;
    }   
    for (int i = 0; i < this->barsRef->getnumBars(); i++) {
        fprintf(fp, "%s", (to_string(this->plArray[i]) + "\n").c_str());
    }
    fclose(fp);

    #endif //PL
}

