
#pragma once
#include "Trade.h"
#include "../utils/memdbg.h"
#include "../myclient/CommonMacros.h"

class BackTester
{
    Bars* const barsRef;
    char* reportPath;
    char* logPath;

    double pl = 0.0;
    double plArray [MAXBARS];
    int currTradeNo = 0;
    int openTrades  = 0;

    std::string tradeLog = "";

    Trade * execTrades [MAXBTTRADES];

    void logTrade(std::string, int, std::string);
    static bool open_path_or_stdout(FILE * &fp, const char * path);
    
    bool closeTrade(int, int, std::string);

    public:

    BackTester(Bars* barsRef, 
               const char* reportPath, 
               const char* logPath);
    ~BackTester();

    //*** INTERFACE ***//
    int  openTrade(int direction, int entryPos, std::string reason, double units = 1.0);
    void close_all_trades(int dir, int exitPos, std::string reason);
    void close_trades_conditionally(int dir, int exitPos, std::string reason, 
                    bool takeProfits, bool stopLosses, int barLim);
    void check_sl_tp(int currPos, double takeProfThresh, double stopLossThresh);
    void update_PnL(int currPos);

    /* logging functions */
    void printResults(const std::string& strat_info);
    void print_PL_data(const std::string outputDir, const std::string str = "PL", const std::string outputExt = ".txt");
};
