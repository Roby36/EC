
#include "Strategy.h"

/** DATAOUTPUT: */ 

void Strategy::print_indicators(const std::string outputDir, const std::string outputExt)
{
    const std::string name = std::string(this->strategy_code + "-");

    m_LocalMin->printIndicator(outputDir, name, outputExt);
    m_LocalMax->printIndicator(outputDir, name, outputExt);
    m_RSI->printIndicator(outputDir, name, outputExt);
    m_Divergence->printIndicator(outputDir, name, outputExt);
    m_LongDivergence->printIndicator(outputDir, name, outputExt);
    m_BollingerBands->printIndicator(outputDir, name, outputExt);
}

void Strategy::print_bars(const std::string outputDir, const std::string outputExt)
{
    const std::string name = std::string(this->strategy_code + "-");
    this->m_instr->bars->printBars(outputDir, name, outputExt);
}

bool Strategy::open_path_or_stdout(FILE * &fp, const char * path) {
    if (path != NULL) {
        Bars::clear_file(path);
        fp = fopen(path, "a");
        if (fp == NULL) {
            fprintf(stderr, "Error opening %s for appending\n", path);
            return false;
        }
    } else fp = stdout;
    return true;
}

void Strategy::print_PL_data(const std::string outputDir, const std::string outputExt)
{
    char * path = (char *) malloc (OUTDIRCHAR);
    strncpy(path, (outputDir + std::string(this->strategy_code + "-" + "PL") + outputExt).c_str(), OUTDIRCHAR);
    Bars::clear_file(path);
    FILE * fp = fopen(path, "a");
    if (fp == NULL) {
        fprintf(stderr, "Error opening PL output file for appending\n");
        return;
    }   
    for (int i = 0; i < this->m_instr->bars->getnumBars(); i++) {
        fprintf(fp, "%s", (std::to_string(this->plArray[i]) + "\n").c_str());
    }
    fclose(fp);
    free(path);
}

std::string Strategy::print_trade(MTrade * trade, int trade_no) 
{
    std::string dir = trade->dir == LONG ? "long" : "short";
    MBar * entry_bar = this->m_instr->bars->getBar(trade->bt_entry_bar);
    MBar * exit_bar  = this->m_instr->bars->getBar(trade->bt_exit_bar);
    return(
          " Trade no: "            + std::to_string(trade_no) + ";"
        + " Direction: "           + dir + ";"
        + " Entry date: "          + std::string   (entry_bar->date_time_str) + ";"
        + " Entry closing price: " + std::to_string(entry_bar->close())       + ";"
        + " Exit date: "           + std::string   (exit_bar->date_time_str)  + ";"
        + " Exit closing price: "  + std::to_string(exit_bar->close())        + ";"
        + " Loss/profit: "         + std::to_string(trade_opening_order_size(trade) * trade->dir *
                                                     (exit_bar->close() - entry_bar->close()))                
        + ";" 
    );
}

void Strategy::print_backtest_results()
{
    // First compute positive and negative trades, at LAST BAR:
    int posTrades = 0; double posBalance = 0.0;
    int negTrades = 0; double negBalance = 0.0;
    double currBal;
    int num_trades = 0;
    MTrade * trade_arr [MAXTRADES]; // store trades belonging to this strategy in this array

    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade* curr_trade = m_tradeData->tradeArr[i];
        /* check if trade belongs to current strategy */
        if (curr_trade->strategy_code != this->strategy_code)
            continue;
        /* update temporary trade array */
        if (num_trades < MAXTRADES)
            trade_arr[num_trades++] = curr_trade;
        /* update positive or negative balance */
        currBal = trade_balance(curr_trade);
        if (currBal > 0) {
            posTrades++;
            posBalance += currBal;
        } else {
            negTrades++;
            negBalance += currBal;
        }
    }

    FILE* fp;
    if (!open_path_or_stdout(fp, this->reportPath)) 
        goto LOG;
    fprintf(fp, "\n%s", this->strat_info().c_str()); /* print all the relevant startegy information first */
    fprintf(fp, "\n STRATEGY REPORT:\nTotal positive trades: %d (%f points); Total negative trades: %d (%f points);\n Net loss/profit: %f\n\n", 
        posTrades, posBalance, negTrades, negBalance, this->pl);
    for (int i = 0; i < num_trades; i++)
        fprintf(fp, "%s Current p&l: %f\n", this->print_trade(trade_arr[i], i).c_str(), plArray[trade_arr[i]->bt_exit_bar]);
    if (this->reportPath != NULL) 
        fclose(fp); 

    LOG:
    if (!open_path_or_stdout(fp, this->logPath)) 
        return;
    std::string tradeLog = "";
    for (int i = 0; i < num_trades; i++) {
        /* Log each trade belonging to this strategy */
        tradeLog += "Opened " + std::to_string(i) + ": " + trade_arr[i]->opening_reason + "\n";
        tradeLog += "Closed " + std::to_string(i) + ": " + trade_arr[i]->closing_reason + "\n";
    }
    fprintf(fp, "\n TRADE LOG:\n%s\n END OF LOG\n\n", tradeLog.c_str());
    if (this->logPath != NULL) 
        fclose(fp);
}

std::string Strategy::strat_info()
{
    std::string entry_cond_str = "";
    std::string exit_cond_str  = "";
    std::string div_type_str   = "";
    std::string rsi_cond_str   = "";
    std::string Boll_cond_str  = "";

    switch (divType) {
        case SHORT_DIV: div_type_str = "Short"; break;
        case LONG_DIV:  div_type_str = "Long";  break;
    }
    switch (RSI_cond) {
        case NONE:       rsi_cond_str = "Right bar of last divergence";  break;
        case LSTAT_RBAR: rsi_cond_str = "Right bar of first divergence"; break;
        case LSTAT_LBAR: rsi_cond_str = "Left bar of first divergence";  break;
    }
    switch (Bollinger_cond) {
        case MIDDLE_BAND: Boll_cond_str = "Cross middle band in profit"; break;
        case OUTER_BANDS: Boll_cond_str = "Cross outer (more extreme) band in profit"; break;
    }
    for (int i = 0; i < MAXENTRYCONDS; i++) {
        EntryConditions cond = this->entry_conditions[i];
        if (cond == ENTRY_CONDITIONS_END) 
            break;
        switch (cond) {
            case DENIED_DIVERGENCE: entry_cond_str += ("\tDenied divergence with maximum negation period " + std::to_string(this->max_neg_period) + 
                                                       " and RSI condition: " + rsi_cond_str + "\n"); break;
            case DOUBLE_DIVERGENCE: entry_cond_str += "\tDouble divergence\n"; break;
        }
    }
    for (int i = 0; i < MAXEXITCONDS; i++) {
        ExitConditions cond = this->exit_conditions[i];
        if (cond == EXIT_CONDITIONS_END)
            break;
        switch (cond) {
            case STOP_LOSS_TAKE_PROFIT: exit_cond_str += ("\tFixed Take profit, stop loss: " + std::to_string(this->take_profit) + ", "
                                                                                             + std::to_string(this->stop_loss) + "\n"); break;
            case OPPOSITE_DIVERGENCE: exit_cond_str += "\tOpposite " + div_type_str + " divergence\n"; break;
            case BOLLINGER_CROSSING:  exit_cond_str += "\tBollinger crossing with condition: " + Boll_cond_str + "\n"; break;
            case NEGATIVE_TRADE_EXPIRATION: exit_cond_str += "\tTrades closed after " + std::to_string(this->expirationBars) + " bars if negative\n"; break;
        }
    }

    if (this->m_instr->bars->getnumBars() == 0) {
        m_logger->str(" Cannot print backtests: no bar data found\n");
        return "";
    }

    return (std::string("Strategy code:\n\t" + this->strategy_code + "\n" +
                        "DataContract:\n\t"  + this->m_instr->dataContract.longName + "\n" +
                            /** NOTE: segfault here when accessing getBar(0) if instrument bars were never loaded! */
                        "Backtesting dates:\n\t" + std::string(this->m_instr->bars->getBar(0)->date_time_str) + "- "
                                                 + std::string(this->m_instr->bars->getBar(curr_bar_index)->date_time_str) + "\n" +  
                        "Indicator settings: " + "\n" + 
                                               + "\t" + "RSI: time-period: " + std::to_string(this->m_RSI->timePeriod) + "\n"
                                               + "\t" + div_type_str + " Divergence with minimum, maximum divergence periods: " + std::to_string(this->m_Divergence->minDivPeriod) + ", " + std::to_string(this->m_Divergence->maxDivPeriod) + "\n" +
                                               + "\t" + "Bollinger Bands: stDevUp, stDevDown, moving average time-period: (" + 
                                                            std::to_string(this->m_BollingerBands->stDevUp)    + ", " + 
                                                            std::to_string(this->m_BollingerBands->stDevDown)  + ", " + 
                                                            std::to_string(this->m_BollingerBands->timePeriod) + ")\n" +
                        "Entry conditions: " + "\n" + entry_cond_str +
                        "Exit conditions:"   + "\n" + exit_cond_str ));
}


