
#pragma once 

#include "BackTester.h"
#include "Indicators.h"
#include "MTrade.h"
#include "MOrders.h"
#include "memdbg.h"
#include "CommonMacros.h"

enum TradingState {
	BACKTESTING,
	LIVE,
	RETRIEVAL
};

enum EntryConditions {
	DENIED_DIVERGENCE,
	DOUBLE_DIVERGENCE,
	ENTRY_CONDITIONS_END
};

enum ExitConditions {
	OPPOSITE_DIVERGENCE,
	BOLLINGER_CROSSING,
	NEGATIVE_TRADE_EXPIRATION,
	STOP_LOSS_TAKE_PROFIT,
	EXIT_CONDITIONS_END
};

enum DivergenceType {
	SHORT,
	LONG
};

enum StatType {
	MAX = -1,
	MIN = 1
};

class Strategy
{
	/** INSTRUMENT: */
	Instrument * /*const*/ m_instr;  
	int curr_bar_index = 0; // bar iteration

	/** LOGGING: */
	Mlogger* const m_logger;

	/** INDICATORS: */
	// Indicators required for each strategy
	Indicators::LocalMin*       m_LocalMin;
	Indicators::LocalMax*       m_LocalMax;
	Indicators::RSI*            m_RSI;
	Indicators::Divergence*		m_Divergence;
	Indicators::LongDivergence* m_LongDivergence;
	Indicators::BollingerBands* m_BollingerBands;
	void compute_indicators();
	void delete_indicators();
	void select_statType(StatType statType, Indicators::LocalMin * &StatIndicator, std::string &stat_point_str);
	void select_divType(DivergenceType divType, Indicators::Divergence * &DivIndicator, std::string &div_point_str);

	/** CONDITIONS: */
	EntryConditions entry_conditions [MAXENTRYCONDS];
    ExitConditions  exit_conditions  [MAXEXITCONDS];
	/* Conditions for denied divergence */
	const DivergenceType divType;
	const int max_neg_period;
	const bool RSI_cond;

	/** ENTRY: */
	bool denied_divergence_general(DivergenceType divType, StatType statType, const int max_neg_period, const bool RSI_cond = true, bool no_open = false);
	bool denied_divergence(DivergenceType divType, const int max_neg_period, const bool RSI_cond);
	bool double_divergence(DivergenceType divType, /* StatType statType, */ bool no_open = false);
	/** EXIT: */
	void opposite_divergence(DivergenceType divType, MTrade_t* curr_trade = NULL);
	void bollinger_crossing(MTrade_t* curr_trade  = NULL);
	void negative_trade_expiration(MTrade_t* curr_trade = NULL);
	void stop_loss_take_profit(MTrade_t* curr_trade = NULL, const double close = -DBL_MAX);
	void check_entry_conditions(DivergenceType divType, const int max_neg_period, const bool RSI_cond);
    void check_exit_conditions(DivergenceType divType, MTrade_t* curr_trade);
	/*const*/ double stop_loss;
	/*const*/ double take_profit;
	/*const*/ int    expirationBars;

	/** TRADES: (live & backtesting) */
	/*const*/ Decimal orderQuant; // fixed order size 
	BackTester* m_bt; // backtesting
	bool check_trade(MTrade_t* curr_trade);
	void openTrade(const std::string strategy,
					const int instrId,
					const Order openingOrder,
					const Order closingOrder,
					const std::string orderRef = "");
	void closeTrade(MTrade_t * curr_trade, 
					const std::string orderRef = "");
	void general_open(const int dir, 
					  const int curr_bar_index, 
					  std::string orderRef = "");

	public:

	Strategy(Instrument* const m_instr,
				TradeData* tradeData,
			// Orders parameters
				const Decimal orderQuant,
			// Backtester parameters
				const char* reportPath, 
				const char* logPath,
			// General strategy parameters
				const double stop_loss, 	 /* S1 = 2.5, S2 = 3.5 */
				const double take_profit, 	 /* S1 = 2.5, S2 = 3.5 */
				const int    expirationBars, /* S1 = 18,  S2 = 63  */
				const std::string strategy_code,
			// Indicator pointers (malloc'd already and simply copied into instance variables)
				Indicators::LocalMin * input_LocalMin,
				Indicators::LocalMax * input_LocalMax,
				Indicators::RSI      * input_RSI,
				Indicators::BollingerBands * input_BollingerBands,
				Indicators::Divergence     * intput_Divergence,
				Indicators::LongDivergence * input_LongDivergence,
			// Condition arrays (to be copied into instance variables)
				EntryConditions input_entry_conditions [],
				ExitConditions  input_exit_conditions  [],
			// Parameters for denied divergence
				const DivergenceType divType = LONG, 
				const int max_neg_period = 14, 
				const bool RSI_cond = true
				);
	~Strategy();

	// Interface to communicate with MClient
	/*const*/ std::string strategy_code;
	TradeData * m_tradeData;
	MTrade_t  * trade2open;
	MTrade_t  * trade2close;
	bool openingTrade  = false;
	bool closingTrade  = false;
	TradingState t_state; 

	void handle_realTimeBar(const double close);
	void handle_barUpdate();
	void print_backtest();
};

