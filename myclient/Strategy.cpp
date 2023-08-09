
#include "Strategy.h"

#ifdef MEMDBG
    bool printed_mem_locs = false;
#endif

Strategy::Strategy(Instrument* const m_instr,
                    TradeData* tradeData,
                //! Orders parameters
                    const Decimal orderQuant,
                //! Backtester parameters
                    const char* reportPath, 
                    const char* logPath,
                //! General strategy parameters
                    const double stop_loss,
                    const double take_profit,
                    const int    expirationBars,
                    const std::string strategy_code,
                //! Indicator pointers (malloc'd already and simply copied into instance variables)
                    Indicators::LocalMin * input_LocalMin,
                    Indicators::LocalMax * input_LocalMax,
                    Indicators::RSI      * input_RSI,
                    Indicators::BollingerBands * input_BollingerBands,
                    Indicators::Divergence     * input_Divergence,
                    Indicators::LongDivergence * input_LongDivergence,
                //! Condition arrays (to be copied into instance variables)
                    EntryConditions input_entry_conditions [],
                    ExitConditions  input_exit_conditions  [],
                // Parameters for denied divergence
                    const DivergenceType divType, 
                    const int max_neg_period, 
                    const bool RSI_cond
                    ) : 
        m_instr(m_instr),
        m_logger(new Mlogger( std::string("../strategies_log/" + strategy_code + ".txt"))),
        divType(divType),
        max_neg_period(max_neg_period),
        RSI_cond(RSI_cond),
        stop_loss(stop_loss),
        take_profit(take_profit),
        expirationBars(expirationBars),
        orderQuant(orderQuant),
        strategy_code(strategy_code),
        m_tradeData(tradeData),
        t_state(RETRIEVAL)
{
    m_bt = new BackTester (m_instr->bars, reportPath, logPath);
    trade2open  = (MTrade_t*) malloc (sizeof(MTrade_t));
    trade2close = (MTrade_t*) malloc (sizeof(MTrade_t));
    // TO BE INITIALIZED above with const pointers!
    m_LocalMin = input_LocalMin;
    m_LocalMax = input_LocalMax;
    m_RSI      = input_RSI;
    m_BollingerBands = input_BollingerBands;
    m_Divergence     = input_Divergence;
    m_LongDivergence = input_LongDivergence;
    // Copying arrays
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
    free(trade2open);
    free(trade2close);
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

//** TRADE OPERATIONS **//

void Strategy::openTrade(const std::string strategy,
                         const int instrId,
                         const Order openingOrder,
                         const Order closingOrder,
                         const std::string orderRef)
{
    MTrade_t * temp_trade = new MTrade_t(m_tradeData->numTrades, m_instr->inst_id,
				this->strategy_code, openingOrder, closingOrder);
    memcpy(trade2open, temp_trade, sizeof(MTrade_t));
	trade2open->openingOrder.orderRef = orderRef;
    this->openingTrade = true; // signal that the trade must be opened
    //! MCllient.cpp must set the flag back to false when trade executed
    delete(temp_trade);
}

void Strategy::closeTrade(MTrade_t * curr_trade,
                          const std::string orderRef)
{
    m_logger->str(std::string("Strategy::closeTrade: closing trade number " + std::to_string(curr_trade->tradeId) + " with orderRef: " + orderRef +  "\n")); 
	trade2close = curr_trade; // no need to memcpy since curr_trade is already a malloc'd pointer
	trade2close->closingOrder.orderRef = orderRef;
    this->closingTrade = true; // signal that the trade must be closed
    //! MCllient.cpp must set the flag back to false when trade executed
}

void Strategy::general_open(const int dir, 
                            const int bar_index, 
                            std::string orderRef)
{
    m_logger->str(std::string("Strategy::general_open: opening trade in dir " + std::to_string(dir) + " at bar index " + std::to_string(bar_index) + " with orderRef: " + orderRef + "\n")); 
    // Determine trade direction
    std::string opening_action;
    std::string closing_action;
    if (dir == 1) {
        opening_action = "BUY";
        closing_action = "SELL";
    } else if (dir == -1) {
        opening_action = "SELL";
        closing_action = "BUY";
    } else {
        m_logger->str("Strategy::general_open(): invalid trade direction\n");
        return;
    } 
    // Consider live trading & backtesting cases
    if (t_state == LIVE) {
        openTrade(strategy_code, m_instr->inst_id,
                MOrders::MarketOrder(opening_action, orderQuant),
                MOrders::MarketOrder(closing_action, orderQuant),
                orderRef);
    } else if (t_state == BACKTESTING) {        
        m_bt->openTrade(dir, bar_index, orderRef);
    }
}

void Strategy::handle_realTimeBar(const double close) 
{
    // Live trading requires stop-loss & take-profit checking here
    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade_t* curr_trade = m_tradeData->tradeArr[i];
        if (!check_trade(curr_trade)) 
            continue;
        stop_loss_take_profit(curr_trade, close);
    }
}

void Strategy::print_backtest()
{
    m_bt->printResults();
}

//** Indicator selection fucntions **//

void Strategy::select_statType(StatType statType, Indicators::LocalMin * &StatIndicator, std::string &stat_point_str)
{
    switch (statType) {
        case MAX: {
            StatIndicator = m_LocalMax;
            stat_point_str = "maximum";
            break; 
            }
        case MIN: {
            StatIndicator  = m_LocalMin;
            stat_point_str = "minimum";
            break;
        }
    }
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
    }
}

//** ENTRY CONDITIONS **//

bool Strategy::denied_divergence_general(DivergenceType divType, StatType statType, const int max_neg_period, const bool RSI_cond, bool no_open) 
{
    // Select desired indicators
    Indicators::LocalMin * StatIndicator;
    Indicators::Divergence * DivIndicator;
    std::string stat_point_str;
    std::string div_point_str;
    select_divType(divType, DivIndicator, div_point_str);
    select_statType(statType, StatIndicator, stat_point_str);
    
    const int startBar = curr_bar_index - 2;
    int barsBack       = 0;
    int currLeftBar;
    int rightBar;

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
    rightBar = startBar - barsBack;
    if (DivIndicator->getIndicatorBar(rightBar)->isDenied)
        return false;
    // Navigate towards the left (root) maximum of the divergence
    currLeftBar = rightBar;
    while (DivIndicator->getIndicatorBar(currLeftBar)->isPresent()) {
        currLeftBar = DivIndicator->getIndicatorBar(currLeftBar)->leftBarIndex;
    }
    /* (3) Current maximum/minimum's RSI greater/smaller than first maximum/minimum's RSI */
    if (RSI_cond &&
        (int)statType * m_RSI->getIndicatorBar(startBar)->RSI > 
        (int)statType * m_RSI->getIndicatorBar(currLeftBar)->RSI)
        return false;
    /* Mark divergence as denied */
    DivIndicator->getIndicatorBar(rightBar)->isDenied = true;
    /* If we have a long divergence, use same function to check if trade would also have been open with short divergence */
    if (divType == LONG && denied_divergence_general(SHORT, statType, max_neg_period, RSI_cond, true)) /* call function without executing any trade */
        div_point_str = "long & short ";
    /* Check if we are testing divergence condition or if trade actually needs to be opened */
    if (!no_open)
        general_open((int)statType, curr_bar_index - 1, 
                std::string("Denied divergence on new local " + stat_point_str +  " on " + 
                        std::string(m_instr->bars->getBar(startBar)->date_time_str) + "; " + div_point_str + "divergence between " + 
                        std::string(m_instr->bars->getBar(currLeftBar)->date_time_str) + " and " + 
                        std::string(m_instr->bars->getBar(rightBar)->date_time_str) + " of entity " + 
                        std::to_string(DivIndicator->getIndicatorBar(rightBar)->divPoints)));
    return true;                  
}

bool Strategy::denied_divergence(DivergenceType divType, const int max_neg_period, const bool RSI_cond) 
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

void Strategy::bollinger_crossing(MTrade_t* curr_trade) 
{
    // bt case
    if (curr_trade == NULL && t_state == BACKTESTING) {
        if (m_BollingerBands->getIndicatorBar(curr_bar_index - 1)->crossUpperDown)
            m_bt->closeTrades (1, curr_bar_index - 1, "Crossed upper Bollinger Bands from above", true, false);
        else if (m_BollingerBands->getIndicatorBar(curr_bar_index - 1)->crossLowerUp)
            m_bt->closeTrades(-1, curr_bar_index - 1, "Crossed lower Bollinger Bands from below", true, false);
    }
    // live trading cases
    else if (curr_trade->openingOrder.action == "BUY" &&
        curr_trade->openingExecution.price < m_instr->bars->getBar(curr_bar_index - 1)->close() && 
        m_BollingerBands->getIndicatorBar(curr_bar_index - 1)->crossUpperDown)
            closeTrade(curr_trade, "Crossed upper Bollinger Bands from above in profit");
    else if (curr_trade->openingOrder.action == "SELL" &&
        curr_trade->openingExecution.price > m_instr->bars->getBar(curr_bar_index - 1)->close() && 
        m_BollingerBands->getIndicatorBar(curr_bar_index - 1)->crossLowerUp)
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

void Strategy::check_entry_conditions(DivergenceType divType, const int max_neg_period, const bool RSI_cond) {
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

void Strategy::check_exit_conditions(DivergenceType divType, MTrade_t* curr_trade) {
    for (int i = 0; i < MAXEXITCONDS; i++) {
        ExitConditions cond = exit_conditions[i];
        // check if we ran out of conditions
        if (cond == EXIT_CONDITIONS_END) break;
        /** CONDITIONSTRINGS: SAME for S1, S2**/
        else if (cond == OPPOSITE_DIVERGENCE)       opposite_divergence(divType, curr_trade);
        else if (cond == BOLLINGER_CROSSING)        bollinger_crossing(curr_trade);
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
    // Iterate through every trade of MClient and check if it belongs to this strategy, with this instrument
    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade_t* curr_trade = m_tradeData->tradeArr[i];
        if (!check_trade(curr_trade)) continue;
        check_exit_conditions(this->divType, curr_trade);
        }
    }
    if (t_state == BACKTESTING) {
        check_exit_conditions(this->divType, NULL);
    }
}
