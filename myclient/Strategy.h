
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

enum RSI_condition {
	LSTAT_LBAR,
	LSTAT_RBAR,
	NONE
};

class Strategy
{
	/** INSTRUMENT: */
	Instrument * const m_instr;  
	int curr_bar_index = 0; 
	TradingState t_state; 

	/** LOGGING: */
	Mlogger* const m_logger;

	/** INDICATORS: */
	// Indicators required for each strategy
	Indicators::LocalMin * const       m_LocalMin;
	Indicators::LocalMax * const       m_LocalMax;
	Indicators::RSI * const            m_RSI;
	Indicators::Divergence * const	   m_Divergence;
	Indicators::LongDivergence * const m_LongDivergence;
	Indicators::BollingerBands * const m_BollingerBands;
	void compute_indicators();
	void delete_indicators();
	void select_statType(StatType statType, Indicators::LocalMin * &StatIndicator, std::string &stat_point_str);
	void select_divType(DivergenceType divType, Indicators::Divergence * &DivIndicator, std::string &div_point_str);

	/** CONDITIONS: */
	EntryConditions entry_conditions [MAXENTRYCONDS];
    ExitConditions  exit_conditions  [MAXEXITCONDS];
	/* Conditions for denied divergence (for now stored as const instance variables) */
	const DivergenceType divType;
	const int max_neg_period;
	const RSI_condition RSI_cond;

	/** ENTRY: */
	bool denied_divergence_general(DivergenceType divType, StatType statType, const int max_neg_period, const RSI_condition RSI_cond = LSTAT_LBAR, bool no_open = false);
	bool denied_divergence(DivergenceType divType, const int max_neg_period, const RSI_condition RSI_cond);
	bool double_divergence(DivergenceType divType, /* StatType statType, */ bool no_open = false);
	/** EXIT: */
	void opposite_divergence(DivergenceType divType, MTrade_t* curr_trade = NULL);
	void bollinger_crossing(MTrade_t* curr_trade  = NULL);
	void negative_trade_expiration(MTrade_t* curr_trade = NULL);
	void stop_loss_take_profit(MTrade_t* curr_trade = NULL, const double close = -DBL_MAX);
	void check_entry_conditions(DivergenceType divType, const int max_neg_period, const RSI_condition RSI_cond);
    void check_exit_conditions(DivergenceType divType, MTrade_t* curr_trade);
	const double stop_loss;
	const double take_profit;
	const int    expirationBars;

	/** TRADES: (live & backtesting) */
	BackTester* m_bt; // backtesting
	bool check_trade(MTrade_t* curr_trade);
	void openTrade(const std::string strategy,
					const int instrId,
					const Order openingOrder,
					const Order closingOrder,
					const std::string orderRef = "");
	void closeTrade(MTrade_t * curr_trade, 
					const std::string orderRef = "");
	void close_opposite_trades(const std::string direction_to_close);
	void general_open(const int dir, 
					  const int bar_index, 
					  std::string orderRef   = "",
					  const double orderSize = 1.0);

	public:

	Strategy(Instrument* const m_instr,
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
				const DivergenceType divType = LONG, 
				const int max_neg_period = 14, 
				const RSI_condition RSI_cond = LSTAT_LBAR,
			// Backtesting results directories
				const std::string bt_report_dir = "../backtesting/reports/", 
				const std::string bt_log_dir    = "../backtesting/logs/",
				const std::string file_ext      = ".txt"
				);
	~Strategy();

	/* Interface to communicate with MClient */
	const std::string strategy_code;
	TradeData * m_tradeData;
	MTrade_t  * trades2open  [MAXOPENTRADES];
	MTrade_t  * trades2close [MAXCLOSETRADES];
	int opening_trades = 0;
	int closing_trades = 0;
	
	void set_trading_state(TradingState t_state);
	void handle_realTimeBar(const double close);
	void handle_barUpdate();

	/*** Data output ***/
	void print_indicators(const std::string outputDir, const std::string outputExt = ".txt");
	void print_bars(const std::string outputDir, const std::string outputExt = ".txt");
	void print_PL_data(const std::string outputDir, const std::string outputExt = ".txt");
	void print_backtest_results();
};

