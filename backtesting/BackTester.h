
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
    int openTrades = 0;

    std::string tradeLog = "";

    Trade * execTrades [MAXBTTRADES];

    void logTrade(std::string, int, std::string);
    static bool open_path_or_stdout(FILE * &fp, const char * path);
   
    public:

    BackTester(Bars* barsRef, 
               const char* reportPath, 
               const char* logPath);
    ~BackTester();

    //*** INTERFACE ***//
    int  openTrade(int, int, std::string, double = 1.0);
    bool closeTrade(int, int, std::string);
    void closeTrades(int, int, std::string, bool = false, bool = false, int = 0);
    void updateTrades(int, double, double);
    void printResults(const std::string& strat_info);
    void print_PL_data(const std::string outputDir, const std::string str = "PL", const std::string outputExt = ".txt");
};
