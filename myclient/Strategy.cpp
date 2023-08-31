
#include "Strategy.h"

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
            // Order size
                const double def_order_size,
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
        default_order_size(def_order_size),
        strategy_code(strategy_code),
        m_tradeData(tradeData)
{
    /* Allocating space for report and log path strings */
    this->reportPath = (char *) malloc (OUTDIRCHAR);
    this->logPath    = (char *) malloc (OUTDIRCHAR);
    strncpy(this->reportPath, (bt_report_dir + strategy_code + std::string("_REPORT") + file_ext).c_str(), OUTDIRCHAR);
    strncpy(this->logPath,    (bt_log_dir    + strategy_code + std::string("_LOG")    + file_ext).c_str(), OUTDIRCHAR);  
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
    delete(m_logger);
    delete_indicators();
    /* Free'ing report and log path strings */
    free(this->reportPath);
    free(this->logPath);
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
    m_LocalMin->computeIndicator();
    m_LocalMax->computeIndicator();
    m_RSI->computeIndicator();
    m_Divergence->computeIndicator();
    m_LongDivergence->computeIndicator();
    m_BollingerBands->computeIndicator();
}

int Strategy::get_instr_id() {
    return this->m_instr->inst_id;
}

void Strategy::set_trading_state(TradingState t_state) {
    this->t_state = t_state;
}

TradingState Strategy::get_trading_state() {
    return this->t_state;
}

//** TRADE OPERATIONS **//

bool Strategy::is_trade_initialized(MTrade * trade)
{
    if (t_state == BACKTESTING)
        return (trade->bt_entry_bar != 0);
    else if (t_state == LIVE)
        return (/* trade->waiting_opening_execution || */ trade->openingExecution != NULL);
    
    return false;
}

bool Strategy::is_trade_open(MTrade * trade)
{
    if (!is_trade_initialized(trade))
        return false;
    if (t_state == BACKTESTING)
        return (trade->bt_exit_bar == trade->bt_entry_bar);
    else if (t_state == LIVE)
        return (!trade->waiting_closing_execution && trade->closingExecution == NULL);
    
    return false;
}

double Strategy::trade_opening_price(MTrade * trade)
{
    if (!is_trade_initialized(trade))
        return 0;
    if (t_state == BACKTESTING)
        return m_instr->bars->getBar(trade->bt_entry_bar)->close();
    else if (t_state == LIVE)
        return trade->openingExecution->price;

    return 0;
}

double Strategy::trade_closing_price(MTrade * trade)
{
    if (!is_trade_initialized(trade))
        return 0;
    if (t_state == BACKTESTING) {
        int closing_bar = is_trade_open(trade) ? curr_bar_index : trade->bt_exit_bar;
        return m_instr->bars->getBar(closing_bar)->close();
    }
    else if (t_state == LIVE)
        return ((trade->closingExecution != NULL)
                ? trade->closingExecution->price : m_instr->bars->getBar(curr_bar_index)->close());
    
    return 0;
}

int Strategy::get_trade_bars(MTrade * trade)
{
    if (!is_trade_initialized(trade))
        return 0;
    if (t_state == BACKTESTING)
        return (curr_bar_index - trade->bt_entry_bar);
    else if (t_state == LIVE) {
        /* Fetch time of execution */
        struct tm exec_time;
        strptime(trade->openingExecution->time.c_str(), "%Y%m%d %H:%M:%S %Z", &exec_time);
        /* Keep iterating back from the current time until we hit the time of execution */
        int bars_back = 0;
        while (difftime(mktime(m_instr->bars->getBar(curr_bar_index - bars_back)->date_time()), 
                        mktime(&exec_time)) > 0) {
            bars_back++;
        }
        return bars_back;
    }
    return 0;
}

double Strategy::trade_opening_order_size(MTrade * trade)
{
    if (!is_trade_initialized(trade))
        return 0;
    if (t_state == BACKTESTING)
        return trade->bt_order_quant;
    else if (t_state == LIVE)
        return trade->openingOrder->totalQuantity;

    return 0;
}

double Strategy::trade_balance(MTrade * trade)
{
    return (trade_opening_order_size(trade) * trade->dir * 
            (trade_closing_price(trade) - trade_opening_price(trade)));
}

void Strategy::close_trades(const TradeDirection direction_to_close, std::string orderRef)
{
    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade* curr_trade = m_tradeData->tradeArr[i];
        if (curr_trade->strategy_code == this->strategy_code && 
            is_trade_open(curr_trade) &&
            curr_trade->dir == direction_to_close)
        {
            this->general_close(curr_trade, orderRef);
        }   
    }
}

void Strategy::update_PnL()
{
    double currPl = 0.0;
    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade* curr_trade = m_tradeData->tradeArr[i];
        if (curr_trade->strategy_code == this->strategy_code)
            currPl += trade_balance(curr_trade);   
    }
    this->pl = currPl;
    this->plArray[curr_bar_index] = this->pl;
}

void Strategy::general_open(const TradeDirection dir, 
                            const int bar_index, 
                            std::string orderRef,
                            const double orderSize)
{
    /* Log function call */
    m_logger->str(std::string("Strategy::general_open: opening trade in dir " + std::to_string(dir) + " at bar index " + std::to_string(bar_index) + " with orderRef: " + orderRef + "\n")); 
    /* Set default order size if needed */
    const double order_size = (orderSize < 0.0) ? this->default_order_size : orderSize;
    /* First mark to close any trade in opposite direction */
    close_trades((TradeDirection)(- dir), "Opened trade in opposite direction");
    /* Check that we haven't exceeded the maximum number of trades that we can open*/
    if (opening_trades == MAXOPENTRADES) {
    m_logger->str(std::string("Strategy::general_open: cannot open trade with orderRef " + orderRef +
                                "; maximum number of trades to open reached\n")); 
        return;
    }
    /* Initialize and fill up new MTrade struct (general parameters) */
    MTrade * new_trade        = new MTrade();
    new_trade->dir            = dir;
    new_trade->strategy_code  = this->strategy_code;
    new_trade->opening_reason = orderRef;
    /** IMPORTANT: Set bar references in EVERY trading mode! */
    new_trade->bt_entry_bar   = bar_index;
    new_trade->bt_exit_bar    = bar_index;
    new_trade->bt_order_quant = order_size;
    /* Live trading parameters */
    if (t_state == LIVE) {
        /* Determine trade direction */
        std::string opening_action = (dir == LONG) ? "BUY" : "SELL";
        std::string closing_action = (dir == LONG) ? "SELL": "BUY" ;
        new_trade->openingOrder   = (Order *) malloc (sizeof(Order));
        new_trade->closingOrder   = (Order *) malloc (sizeof(Order));
        Order opening_order = MOrders::MarketOrder(opening_action, doubleToDecimal(order_size)
        /** CORRUPTION: Order struct corrupted --> Error. Id: 669, Code: 579, Msg: Invalid symbol in string - 
            std::string(this->strategy_code + std::string(": ") + orderRef)  
        */
        );
        Order closing_order = MOrders::MarketOrder(closing_action, doubleToDecimal(order_size));
        memcpy(new_trade->openingOrder, &opening_order, sizeof(Order));
        memcpy(new_trade->closingOrder, &closing_order, sizeof(Order));
        /** BUSERROR:!!!
        * new_trade->openingOrder = MOrders::MarketOrder(opening_action, doubleToDecimal(order_size), this->strategy_code + ": "+ orderRef);
        * new_trade->closingOrder = MOrders::MarketOrder(closing_action, doubleToDecimal(order_size));
        */ 
        new_trade->waiting_opening_execution = true;
    }
    /* Add trade to array of trades to open*/
    trades2open[opening_trades++] = new_trade;
}

void Strategy::general_close(MTrade * trade, const std::string orderRef)
{
    /* Check input trade */
    if (trade->strategy_code != this->strategy_code) {
        m_logger->str(std::string("Strategy::general_close: cannot close trade with tradeId " + std::to_string(trade->tradeId) +
                                  "; trade has strategy code " + trade->strategy_code + " but this->strategy_code = " + this->strategy_code + "\n")); 
        return;
    }
    if (!is_trade_open(trade)) {
        m_logger->str(std::string("Strategy::general_close: cannot close trade with tradeId " + std::to_string(trade->tradeId) +
                                  "; trade is already closed or waiting for closing execution\n")); 
        return;
    }
    if (closing_trades == MAXCLOSETRADES) {
        m_logger->str(std::string("Strategy::general_close: cannot close trade with tradeId " + std::to_string(trade->tradeId) +
                                  "; maximum number of trades to close reached\n")); 
        return;
    }
    m_logger->str(std::string("Strategy::closeTrade: closing trade number " + std::to_string(trade->tradeId) + " with orderRef: " + orderRef +  "\n")); 
    /* fill up necessary parameters */
    trade->closing_reason = orderRef;
    /** IMPORTANT: Set bar references in EVERY trading mode! */
    trade->bt_exit_bar = curr_bar_index;
    if (t_state == LIVE) {
        /** NOTE: not adding orderRef's to avoid corrupting Order struct
          trade->closingOrder->orderRef    = std::string(this->strategy_code + ": "+ orderRef);
        */
        trade->waiting_closing_execution = true;
    }
    /* Add trade to array of trades to close*/
	trades2close[closing_trades++] = trade; /* no need to memcpy since curr_trade is already a malloc'd pointer */
}

void Strategy::handle_realTimeBar(const double close) 
{
    // Live trading requires stop-loss & take-profit checking here
    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade* curr_trade = m_tradeData->tradeArr[i];
        if (curr_trade->strategy_code == this->strategy_code && 
            is_trade_open(curr_trade)) 
            stop_loss_take_profit(curr_trade, close);
    }
}

//** Indicator selection functions **//

void Strategy::select_statType(StatType statType, Indicators::LocalMin * &StatIndicator, std::string &stat_point_str)
{
    StatIndicator  = (statType == MAX) ? m_LocalMax : m_LocalMin;
    stat_point_str = (statType == MAX) ? "maximum"  : "minimum";
}

void Strategy::select_divType(DivergenceType divType, Indicators::Divergence * &DivIndicator, std::string &div_point_str)
{
    switch (divType) {
        case SHORT_DIV: {
            DivIndicator  = m_Divergence; 
            div_point_str = "short ";
            break;
        }
        case LONG_DIV: {
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
    
    const int startBar = curr_bar_index - 1; /* candidate new maximum/minimum denying divergence */ 
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
    if (divType == LONG_DIV && denied_divergence_general(SHORT_DIV, statType, max_neg_period, RSI_cond, true)) /* call function without executing any trade */
        div_point_str = "long & short ";
    /* Check if we are testing divergence condition or if trade actually needs to be opened */
    if (!no_open)
        general_open((TradeDirection) statType, curr_bar_index, 
                std::string("Denied divergence on new local " + stat_point_str +  " on " + 
                        std::string(m_instr->bars->getBar(startBar)->date_time_str) + "; " + div_point_str + "divergence between most_left_bar " + 
                        std::string(m_instr->bars->getBar(most_left_bar)->date_time_str) + " and most_right_bar " + 
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
    if (DivIndicator->getIndicatorBar(curr_bar_index - 1)->divPoints < 3)
        return false;
    /* Check if this also applies to short divergernce*/
    if (divType == LONG_DIV && double_divergence(SHORT_DIV, true)) /* call without executing any trade */
        div_point_str = "long & short ";
    // Go long if we are on a minimum, short if we are on a maximum
    if (!no_open) /** NOTE: use curr_bar_index as opening bar_index, 1 bar after bar index where divergence occurred! */
        general_open((TradeDirection) DivIndicator->getIndicatorBar(curr_bar_index - 1)->m, curr_bar_index,
                div_point_str + DivIndicator->getIndicatorBar(curr_bar_index - 1)->logString());
    return true;
}

/** EXIT CONDITIONS **/

void Strategy::opposite_divergence(DivergenceType divType, MTrade* curr_trade)
{
    /* Select appropriate divergence indicator */
    Indicators::Divergence * DivIndicator;
    std::string div_point_str;
    select_divType(divType, DivIndicator, div_point_str);
    /* Check if we have a "long & short divergence" */
    if (divType == LONG_DIV && m_Divergence->getIndicatorBar(curr_bar_index - 1)->isPresent())
        div_point_str = "long & short ";
    /* Close trade if divergence m-value corresponds with negative trade direction */
    if (DivIndicator->getIndicatorBar(curr_bar_index - 1)->m == - curr_trade->dir)
        general_close(curr_trade, std::string("Opposite (not necessarily denied) " + div_point_str + "divergence"));
}

void Strategy::bollinger_crossing(MTrade* curr_trade, BOLLINGER_CONDITION Bollinger_cond) 
{
    /* Close trade only if positive balance */
    if (trade_balance(curr_trade) < 0)
        return;
    /* Fill up conditions based on input Bollinger Condition */
    bool close_long_condition, close_short_condition;
    switch (Bollinger_cond) {
        case OUTER_BANDS: {
            close_long_condition  = m_BollingerBands->getIndicatorBar(curr_bar_index)->crossUpperDown;
            close_short_condition = m_BollingerBands->getIndicatorBar(curr_bar_index)->crossLowerUp;
            break;
        }
        case MIDDLE_BAND: {
            close_long_condition  = m_BollingerBands->getIndicatorBar(curr_bar_index)->crossMiddleDown;
            close_short_condition = m_BollingerBands->getIndicatorBar(curr_bar_index)->crossMiddleUp;
            break;
        }
    }
    /* Check if we met any Bollinger condition */
    if (close_long_condition)
        close_trades(LONG, "Crossed upper Bollinger Bands from above in profit");
    else if (close_short_condition)
        close_trades(SHORT, "Crossed lower Bollinger Bands from below in profit");
}

void Strategy::negative_trade_expiration(MTrade* curr_trade)
{
    /* Close trade only if negative balance */
    if (trade_balance(curr_trade) > 0)
        return;
    /* Check bars elapsed since trade was opened */
    if (get_trade_bars(curr_trade) > this->expirationBars) 
    {
        general_close(curr_trade, "Negative after expiration time");
    }
}

void Strategy::stop_loss_take_profit(MTrade* curr_trade, const double close) 
{
    /* If not called with realtime bar data (where close != -DBL_MAX) 
       then set close price to current bar index */
    double closing_price = (close == - DBL_MAX) ? trade_closing_price(curr_trade) : close;
    std::string reason   = (close == - DBL_MAX) ? "(handle_barUpdate)" : "(handle_realTimeBar)";
    double opening_price = trade_opening_price(curr_trade);
    double rel_balance   = curr_trade->dir * (closing_price - opening_price);
    
    if ( 100 * rel_balance / opening_price > take_profit ||
        -100 * rel_balance / opening_price > stop_loss) 
    {
        general_close(curr_trade, std::string("Reached take profit / stop loss " + reason));
    }
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

void Strategy::check_exit_conditions(MTrade* curr_trade, DivergenceType divType, const BOLLINGER_CONDITION Bollinger_cond) {
    for (int i = 0; i < MAXEXITCONDS; i++) {
        ExitConditions cond = exit_conditions[i];
        // check if we ran out of conditions
        if (cond == EXIT_CONDITIONS_END) break;
        /** CONDITIONSTRINGS: SAME for S1, S2**/
        else if (cond == OPPOSITE_DIVERGENCE)       opposite_divergence(divType, curr_trade);
        else if (cond == BOLLINGER_CROSSING)        bollinger_crossing(curr_trade, Bollinger_cond);
        else if (cond == NEGATIVE_TRADE_EXPIRATION) negative_trade_expiration(curr_trade);
        else if (cond == STOP_LOSS_TAKE_PROFIT)     stop_loss_take_profit(curr_trade, - DBL_MAX);
        /** Other possible exit conditions  ... */
    }
}

/*** Only function with hard-coded (DivergenceType divType, const int max_neg_period, const bool RSI_cond) parameters ***/

void Strategy::handle_barUpdate() {
    // Update & verify bar position (including 1 bar iterating back)
    if ((curr_bar_index = m_instr->bars->getnumBars() - 1) < 2) return;
    // First update indicators to account for new bar
    compute_indicators();
    //** ENTRY CONDITIONS (live & backtesting) **//
    check_entry_conditions(this->divType, this->max_neg_period, this->RSI_cond);
    //** EXIT CONDITIONS **//
    for (int i = 0; i < m_tradeData->numTrades; i++) {
        MTrade* curr_trade = m_tradeData->tradeArr[i];
        if (curr_trade->strategy_code == this->strategy_code && 
            is_trade_open(curr_trade)) 
        {
            check_exit_conditions(curr_trade, this->divType, this->Bollinger_cond);
        }  
    }
    /* update PnL balance */
    this->update_PnL(); /* update balance after each bar update */
}

