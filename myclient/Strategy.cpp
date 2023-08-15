
#include "Strategy.h"

#ifdef MEMDBG
    bool printed_mem_locs = false;
#endif

Strategy::Strategy(Instrument* const m_instr,
				TradeData* tradeData,
			// General strategy parameters
				const double stop_loss, 	 /* S1 = 2.5, S2 = 3.5 */
				const double take_profit, 	 /* S1 = 2.5, S2 = 3.5 */
				const int    expirationBars, /* S1 = 18,  S2 = 63  */
				const std::string strategy_code,
            // Condition arrays (to be copied into instance variables)
                EntryConditions input_entry_conditions [],
                ExitConditions  input_exit_conditions  [],
			// Indicator pointers (malloc'd already and simply copied into instance variables)
				Indicators::LocalMin * input_LocalMin,
				Indicators::LocalMax * input_LocalMax,
				Indicators::RSI      * input_RSI,
				Indicators::BollingerBands * input_BollingerBands,
				Indicators::Divergence     * input_Divergence,
				Indicators::LongDivergence * input_LongDivergence,
            // Parameters for denied divergence
				const DivergenceType divType, 
				const int max_neg_period, 
				const RSI_condition RSI_cond,
                const BOLLINGER_CONDITION Bollinger_cond,
			// Backtesting results directories
				const std::string bt_report_dir, 
				const std::string bt_log_dir,
                const std::string file_ext
				) :   
        m_instr(m_instr),
        t_state(RETRIEVAL),
        m_logger(new Mlogger( std::string("../strategies_log/" + strategy_code + ".txt"))),
        m_LocalMin(input_LocalMin), m_LocalMax(input_LocalMax), m_RSI(input_RSI),
        m_Divergence(input_Divergence), m_LongDivergence(input_LongDivergence), m_BollingerBands(input_BollingerBands), 
        divType(divType),
        max_neg_period(max_neg_period),
        RSI_cond(RSI_cond),
        Bollinger_cond(Bollinger_cond),
        stop_loss(stop_loss),
        take_profit(take_profit),
        expirationBars(expirationBars),
        strategy_code(strategy_code),
        m_tradeData(tradeData)
{
    /* Initializing backtester */
    m_bt = new BackTester (m_instr->bars, 
                (bt_report_dir + strategy_code + file_ext).c_str(), 
                (bt_log_dir    + strategy_code + file_ext).c_str());
    /* Initializing live trade structures */
    for (int i = 0; i < MAXOPENTRADES; i++) {
        trades2open[i] = (MTrade_t*) malloc (sizeof(MTrade_t));
    }
    for (int i = 0; i < MAXCLOSETRADES; i++) {
        trades2close[i] = (MTrade_t*) malloc (sizeof(MTrade_t));    
    }
    /* Copying condition arrays */
    for (int i = 0; i < MAXENTRYCONDS; i++) {
        EntryConditions cond = input_entry_conditions[i];
        entry_conditions[i]  = cond; // copy condition
        if (cond == ENTRY_CONDITIONS_END) // exit loop as soon as empty condition met
            break;
    }
    for (int i = 0; i < MAXEXITCONDS; i++) {
        ExitConditions cond = input_exit_conditions[i];
        exit_conditions[i]   = cond;
        if (cond == EXIT_CONDITIONS_END)
            break;
    }
}

Strategy::~Strategy()
{
    delete(m_bt);
    delete(m_logger);
    delete_indicators();
    /* Free'ing live trade structures */
    for (int i = 0; i < MAXOPENTRADES; i++) {
        free(trades2open[i]);
    }
    for (int i = 0; i < MAXCLOSETRADES; i++) {
        free(trades2close[i]);
    }
}

void Strategy::delete_indicators()
{
    delete(m_Divergence);
    delete(m_LongDivergence);
    delete(m_BollingerBands);
    delete(m_LocalMax);
    delete(m_LocalMin);
    delete(m_RSI);
}

void Strategy::compute_indicators()
{
#ifdef MEMDBG
    if (!printed_mem_locs) {
        printf("Strategy::compute_indicators sp:"); get_sp();
        printf("S1 location: %p\n",         (void*) this);
        printf("m_LocalMin location: %p\n", (void*) m_LocalMin);
        printf("m_LocalMax location: %p\n", (void*) m_LocalMax);
        printed_mem_locs = true;
    }
#endif
    m_LocalMin->computeIndicator();
    m_LocalMax->computeIndicator();
    m_RSI->computeIndicator();
    m_Divergence->computeIndicator();
    m_LongDivergence->computeIndicator();
    m_BollingerBands->computeIndicator();
}

/*** DATA OUTPUT ***/

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

void Strategy::print_PL_data(const std::string outputDir, const std::string outputExt)
{
    m_bt->print_PL_data(outputDir, std::string(this->strategy_code + "-" + "PL"), outputExt);
}

void Strategy::print_backtest_results()
{
    m_bt->printResults(this->strat_info());
}

void Strategy::set_trading_state(TradingState t_state) {
    this->t_state = t_state;
}

//** TRADE OPERATIONS **//

void Strategy::openTrade(const std::string strategy,
                         const int instrId,
                         const Order openingOrder,
                         const Order closingOrder,
                         const std::string orderRef)
{
    if (opening_trades == MAXOPENTRADES) {
        m_logger->str(std::string("Strategy::openTrade: cannot open trade with orderRef " + orderRef +
                                  "; maximum number of trades to open reached\n")); 
        return;
    }
    MTrade_t * temp_trade = new MTrade_t(m_tradeData->numTrades, m_instr->inst_id,
				this->strategy_code, openingOrder, closingOrder);
    memcpy(trades2open[opening_trades], temp_trade, sizeof(MTrade_t));
	trades2open[opening_trades]->openingOrder.orderRef = orderRef;
    opening_trades++;
    delete(temp_trade);
}

void Strategy::closeTrade(MTrade_t * curr_trade, const std::string orderRef)
{
    if (closing_trades == MAXCLOSETRADES) {
        m_logger->str(std::string("Strategy::closeTrade: cannot close trade with tradeId " + std::to_string(curr_trade->tradeId) +
                                  "; maximum number of trades to close reached\n")); 
        return;
    }
    m_logger->str(std::string("Strategy::closeTrade: closing trade number " + std::to_string(curr_trade->tradeId) + " with orderRef: " + orderRef +  "\n")); 
	trades2close[closing_trades] = curr_trade; /* no need to memcpy since curr_trade is already a malloc'd pointer */
	trades2close[closing_trades]->closingOrder.orderRef = orderRef;
    closing_trades++;
}

void Strategy::close_opposite_trades(const std::string direction_to_close)
{
    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade_t* curr_trade = m_tradeData->tradeArr[i];
        if (check_trade(curr_trade) && 
            curr_trade->openingOrder.action == direction_to_close)
        {
            this->closeTrade(curr_trade, "Opened trade in opposite direction");
        }   
    }
}

void Strategy::general_open(const int dir, 
                            const int bar_index, 
                            std::string orderRef,
                            const double orderSize)
{
    m_logger->str(std::string("Strategy::general_open: opening trade in dir " + std::to_string(dir) + " at bar index " + std::to_string(bar_index) + " with orderRef: " + orderRef + "\n")); 
    /* Determine trade direction */
    if ((dir != 1) && (dir != -1)) {
        m_logger->str("Strategy::general_open(): invalid trade direction\n");
        return;
    }
    std::string opening_action = (dir == 1) ? "BUY" : "SELL";
    std::string closing_action = (dir == 1) ? "SELL": "BUY" ;
    /* Consider live trading & backtesting cases */
    if (t_state == LIVE) {
        /* First mark to close any trade in opposite direction */
        close_opposite_trades(closing_action);
        openTrade(strategy_code, m_instr->inst_id,
                MOrders::MarketOrder(opening_action, orderSize),
                MOrders::MarketOrder(closing_action, orderSize), orderRef);
    } else if (t_state == BACKTESTING) {        
        m_bt->openTrade(dir, bar_index, orderRef, orderSize);
    }
}

void Strategy::handle_realTimeBar(const double close) 
{
    // Live trading requires stop-loss & take-profit checking here
    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade_t* curr_trade = m_tradeData->tradeArr[i];
        if (check_trade(curr_trade)) 
            stop_loss_take_profit(curr_trade, close);
    }
}

//** Indicator selection fucntions **//

void Strategy::select_statType(StatType statType, Indicators::LocalMin * &StatIndicator, std::string &stat_point_str)
{
    StatIndicator  = (statType == MAX) ? m_LocalMax : m_LocalMin;
    stat_point_str = (statType == MAX) ? "maximum"  : "minimum";
}

void Strategy::select_divType(DivergenceType divType, Indicators::Divergence * &DivIndicator, std::string &div_point_str)
{
    switch (divType) {
        case SHORT: {
            DivIndicator  = m_Divergence; 
            div_point_str = "short ";
            break;
        }
        case LONG: {
            DivIndicator  = m_LongDivergence; 
            div_point_str = "long ";
            break;
        }
        /* There may be more divergence types here */
    }
}

//** ENTRY CONDITIONS **//

bool Strategy::denied_divergence_general(DivergenceType divType, StatType statType, const int max_neg_period, const RSI_condition RSI_cond, bool no_open) 
{
    // Select desired indicators
    Indicators::LocalMin * StatIndicator;
    Indicators::Divergence * DivIndicator;
    std::string stat_point_str;
    std::string div_point_str;
    select_divType(divType, DivIndicator, div_point_str);
    select_statType(statType, StatIndicator, stat_point_str);
    
    const int startBar = curr_bar_index - 2; /* candidate new maximum/minimum denying divergence */ 
    int barsBack = 0;
    int most_left_bar;  /* most left bar at the start of divergence sequence */
    int last_div_bar;   /* most left bar still marked with a divergence */
    int most_right_bar; /* last bar marked with divergence (which we are trying to mark as "denied") */

    /*** CONDITIONS: ***/
    /* (1) We are on a maximum/minimum without divergence */
    if (!StatIndicator->getIndicatorBar(startBar)->isPresent() ||
        DivIndicator->getIndicatorBar(startBar)->isPresent()) 
        return false;
    /* (2) We are on a new maximum/minimum since the previous divergence (located on a maximum/minimum) */
    while (DivIndicator->getIndicatorBar(startBar - barsBack)->m != (int)statType) {
        if (startBar - (++barsBack) < 0 ||  // move one bar back
            barsBack > max_neg_period ||  // check max negation period, 
            /* (first_stat_neg && barsBack > 1 &&  // if we find a stationary point without divergence, then we can't be on first maximum/minimum after a divergence 
                StatIndicator->getIndicatorBar(startBar - barsBack + 1)->isPresent()) ||  // not a particularly useful parameter */
            (int)statType * m_instr->bars->getBar(startBar)->close() > /* check whether we are on new maximum/minimum*/
            (int)statType * m_instr->bars->getBar(startBar - barsBack)->close())
            return false;
    }
    /* Check if this divergence was denied already */
    most_right_bar = startBar - barsBack;
    if (DivIndicator->getIndicatorBar(most_right_bar)->isDenied)
        return false;
    /* Navigate towards the left (root) maximum of the divergence */
    most_left_bar = most_right_bar;
    while (DivIndicator->getIndicatorBar(most_left_bar)->isPresent()) {
        last_div_bar  = most_left_bar;
        most_left_bar = DivIndicator->getIndicatorBar(most_left_bar)->leftBarIndex;
    }
    /* (3) check RSI condition */
    int comp_bar;
    switch (RSI_cond) {
        case LSTAT_LBAR: comp_bar = most_left_bar;  break;
        case LSTAT_RBAR: comp_bar = last_div_bar;   break;
        default:         comp_bar = most_right_bar; break;
    }
    if ((int)statType * m_RSI->getIndicatorBar(startBar)->RSI > 
        (int)statType * m_RSI->getIndicatorBar(comp_bar)->RSI)
        return false;
    /* Mark divergence as denied */
    DivIndicator->getIndicatorBar(most_right_bar)->isDenied = true;
    /* If we have a long divergence, use same function to check if trade would also have been open with short divergence */
    if (divType == LONG && denied_divergence_general(SHORT, statType, max_neg_period, RSI_cond, true)) /* call function without executing any trade */
        div_point_str = "long & short ";
    /* Check if we are testing divergence condition or if trade actually needs to be opened */
    if (!no_open)
        general_open((int)statType, curr_bar_index - 1, 
                std::string("Denied divergence on new local " + stat_point_str +  " on " + 
                        std::string(m_instr->bars->getBar(startBar)->date_time_str) + "; " + div_point_str + "divergence between most_left_bar " + 
                        std::string(m_instr->bars->getBar(most_left_bar)->date_time_str) + " and most_right_bar" + 
                        std::string(m_instr->bars->getBar(most_right_bar)->date_time_str) + " of entity " + 
                        std::to_string(DivIndicator->getIndicatorBar(most_right_bar)->divPoints)) + " with absolute % change per bar " +
                        std::to_string(DivIndicator->getIndicatorBar(most_right_bar)->abs_perc_change_per_bar));
    return true;                  
}

bool Strategy::denied_divergence(DivergenceType divType, const int max_neg_period, const RSI_condition RSI_cond) 
{
    return (denied_divergence_general(divType, MAX, max_neg_period, RSI_cond, false) ||
            denied_divergence_general(divType, MIN, max_neg_period, RSI_cond, false));
}

bool Strategy::double_divergence(DivergenceType divType, /* StatType statType, */ bool no_open)
{
    // Select appropriate divergence indicator
    Indicators::Divergence * DivIndicator;
    std::string div_point_str;
    select_divType(divType, DivIndicator, div_point_str);

    /*** CONDITIONS: ***/
    /* (1) Check for at least double divergence on PREVIOUS bar */
    if (DivIndicator->getIndicatorBar(curr_bar_index - 2)->divPoints < 3)
        return false;
    /* Check if this also applies to short divergernce*/
    if (divType == LONG && double_divergence(SHORT, true)) /* call without executing any trade */
        div_point_str = "long & short ";
    // Go long if we are on a minimum, short if we are on a maximum
    if (!no_open)
        general_open(DivIndicator->getIndicatorBar(curr_bar_index - 2)->m, curr_bar_index - 1,
                div_point_str + DivIndicator->getIndicatorBar(curr_bar_index - 2)->logString());
    return true;
}

bool Strategy::check_trade(MTrade_t* curr_trade)
{
    return (curr_trade->instr_id == m_instr->inst_id    &&
            curr_trade->strategy == this->strategy_code &&
            curr_trade->isOpen);
}

/** EXIT CONDITIONS **/

void Strategy::opposite_divergence(DivergenceType divType, MTrade_t* curr_trade)
{
    // Select appropriate divergence indicator
    Indicators::Divergence * DivIndicator;
    std::string div_point_str;
    select_divType(divType, DivIndicator, div_point_str);
    // Check if we have a "long & short divergence"
    if (divType == LONG && m_Divergence->getIndicatorBar(curr_bar_index - 2)->isPresent())
        div_point_str = "long & short ";
    // Check backtesting & livetrading cases
    if (curr_trade == NULL && t_state == BACKTESTING) 
        m_bt->closeTrades((-1) * DivIndicator->getIndicatorBar(curr_bar_index - 2)->m, curr_bar_index - 1, 
        std::string("Opposite (not necessarily denied) " + div_point_str + "divergence on previous bar"));
    // live trading case
    else if (DivIndicator->getIndicatorBar(curr_bar_index - 2)->isPresent() &&
        ((DivIndicator->getIndicatorBar(curr_bar_index - 2)->m == 1 && curr_trade->openingOrder.action == "SELL") || 
         (DivIndicator->getIndicatorBar(curr_bar_index - 2)->m == -1 && curr_trade->openingOrder.action == "BUY")))
        closeTrade(curr_trade, std::string("Opposite (not necessarily denied) " + div_point_str + "divergence"));
}

void Strategy::bollinger_crossing(MTrade_t* curr_trade, BOLLINGER_CONDITION Bollinger_cond) 
{
    bool close_long_condition, close_short_condition;
    switch (Bollinger_cond) {
        case OUTER_BANDS: {
            close_long_condition  = m_BollingerBands->getIndicatorBar(curr_bar_index - 1)->crossUpperDown;
            close_short_condition = m_BollingerBands->getIndicatorBar(curr_bar_index - 1)->crossLowerUp;
            break;
        }
        case MIDDLE_BAND: {
            close_long_condition  = m_BollingerBands->getIndicatorBar(curr_bar_index - 1)->crossMiddleDown;
            close_short_condition = m_BollingerBands->getIndicatorBar(curr_bar_index - 1)->crossMiddleUp;
            break;
        }
    }
    /* bt case: already handles profit-checking */
    if (curr_trade == NULL && t_state == BACKTESTING) {
        if (close_long_condition)
            m_bt->closeTrades (1, curr_bar_index - 1, "Crossed upper Bollinger Bands from above", true, false);
        else if (close_short_condition)
            m_bt->closeTrades(-1, curr_bar_index - 1, "Crossed lower Bollinger Bands from below", true, false);
    }
    /* live trading cases: require profit-checking here */
    else if (curr_trade->openingOrder.action == "BUY" &&
        curr_trade->openingExecution.price < m_instr->bars->getBar(curr_bar_index - 1)->close() && 
        close_long_condition)
            closeTrade(curr_trade, "Crossed upper Bollinger Bands from above in profit");
    else if (curr_trade->openingOrder.action == "SELL" &&
        curr_trade->openingExecution.price > m_instr->bars->getBar(curr_bar_index - 1)->close() && 
        close_short_condition)
            closeTrade(curr_trade, "Crossed lower Bollinger Bands from below in profit");
}

void Strategy::negative_trade_expiration(MTrade_t* curr_trade)
{
    // bt case
    if (curr_trade == NULL && t_state == BACKTESTING) {
        m_bt->closeTrades (1, curr_bar_index - 1, "Negative after expiration time", false, true, expirationBars);
        m_bt->closeTrades(-1, curr_bar_index - 1, "Negative after expiration time", false, true, expirationBars);
        return;
    }
    // live trading: Ignore currently positive trades
    if (((curr_trade->openingOrder.action == "BUY" &&
          curr_trade->openingExecution.price < m_instr->bars->getBar(curr_bar_index - 1)->close()) ||
         (curr_trade->openingOrder.action == "SELL" &&
          curr_trade->openingExecution.price > m_instr->bars->getBar(curr_bar_index - 1)->close())))
        return;
    //! Better to use time to check time expiration since bar numbers will change
    int tot_seconds = expirationBars * m_instr->sec_barSize;
    //! Extract time of execution, assuming the following format
    struct tm exec_time;
    strptime(curr_trade->openingExecution.time.c_str(), "%Y%m%d %H:%M:%S %Z", &exec_time);
    if (difftime(mktime(m_instr->bars->getBar(curr_bar_index - 1)->date_time()), mktime(&exec_time)) > tot_seconds)
        closeTrade(curr_trade, "Negative after expiration");
}

void Strategy::stop_loss_take_profit(MTrade_t* curr_trade, const double close) 
{
    // Ignore if not called from within handle_relatimeBar (where close != -DBL_MAX)
    if (close == -DBL_MAX && t_state == LIVE)
        return;
    // bt case
    if (curr_trade == NULL && t_state == BACKTESTING) {
        m_bt->updateTrades(curr_bar_index - 1, take_profit, stop_loss);
        return;
    }
    // live trading case
    const double openPrice = curr_trade->openingExecution.price;
    const double currPrice = close;
    if ((curr_trade->openingOrder.action == "BUY" &&
            (100 * (currPrice - openPrice) / openPrice > take_profit ||
             100 * (currPrice - openPrice) / openPrice < - stop_loss)) ||
        (curr_trade->openingOrder.action == "SELL" &&
           (-100 * (currPrice - openPrice) / openPrice > take_profit ||
            -100 * (currPrice - openPrice) / openPrice < - stop_loss)))
        closeTrade(curr_trade, "Reached take profit / stop loss");
}

//** CONDITION CHECKS **//

void Strategy::check_entry_conditions(DivergenceType divType, const int max_neg_period, const RSI_condition RSI_cond) {
    for (int i = 0; i < MAXENTRYCONDS; i++) {
        EntryConditions cond = entry_conditions[i];
        // check if we ran out of conditions
        if (cond == ENTRY_CONDITIONS_END) break;
        /** CONDITIONSTRINGS: **/
        else if (cond == DOUBLE_DIVERGENCE) double_divergence(divType, false); /* S2 */
        else if (cond == DENIED_DIVERGENCE) denied_divergence(divType, max_neg_period, RSI_cond); /* S1 */
        /** Other possible entry conditions  ... */
    }
}

void Strategy::check_exit_conditions(MTrade_t* curr_trade, DivergenceType divType, const BOLLINGER_CONDITION Bollinger_cond) {
    for (int i = 0; i < MAXEXITCONDS; i++) {
        ExitConditions cond = exit_conditions[i];
        // check if we ran out of conditions
        if (cond == EXIT_CONDITIONS_END) break;
        /** CONDITIONSTRINGS: SAME for S1, S2**/
        else if (cond == OPPOSITE_DIVERGENCE)       opposite_divergence(divType, curr_trade);
        else if (cond == BOLLINGER_CROSSING)        bollinger_crossing(curr_trade, Bollinger_cond);
        else if (cond == NEGATIVE_TRADE_EXPIRATION) negative_trade_expiration(curr_trade);
        else if (cond == STOP_LOSS_TAKE_PROFIT)     stop_loss_take_profit(curr_trade);
        /** Other possible exit conditions  ... */
    }
}


/*** Only function with hard-coded (DivergenceType divType, const int max_neg_period, const bool RSI_cond) parameters ***/

void Strategy::handle_barUpdate() {
    // Update & verify bar position (including 1 bar iterating back)
    if ((curr_bar_index = m_instr->bars->getnumBars()) < 3) return;
    // First update indicators to account for new bar
    compute_indicators();
    //** ENTRY CONDITIONS (live & backtesting) **//
    check_entry_conditions(this->divType, this->max_neg_period, this->RSI_cond);
    //** EXIT CONDITIONS **//
    if (t_state == LIVE) {
    /* Iterate through every trade of MClient and check if it belongs to this strategy, with this instrument */
        for (int i = 0; i < m_tradeData->numTrades; i++) {
            MTrade_t* curr_trade = m_tradeData->tradeArr[i];
            if (check_trade(curr_trade))
                check_exit_conditions(curr_trade, this->divType, this->Bollinger_cond);
            }
    }
    if (t_state == BACKTESTING) {
        check_exit_conditions( NULL, this->divType, this->Bollinger_cond);
    }
}

/** LOGGING: **/

std::string Strategy::strat_info()
{
    std::string entry_cond_str = "";
    std::string exit_cond_str  = "";
    std::string div_type_str   = "";
    std::string rsi_cond_str   = "";
    std::string Boll_cond_str  = "";

    switch (divType) {
        case SHORT: div_type_str = "Short"; break;
        case LONG:  div_type_str = "Long";  break;
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

    return (std::string("Strategy code:\n\t" + this->strategy_code + "\n" +
                        "DataContract:\n\t"  + this->m_instr->dataContract.longName + "\n" +
                        "Backtesting dates:\n\t" + std::string(this->m_instr->bars->getBar(0)->date_time_str) + "- "
                                                 + std::string(this->m_instr->bars->getBar(curr_bar_index - 1)->date_time_str) + "\n" +  
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
