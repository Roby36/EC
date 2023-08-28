
#pragma once 

#include "Indicators.h"
#include "MTrade.h"
#include "MOrders.h"
#include "memdbg.h"
#include "CommonMacros.h"
#include "CommonEnums.h"

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
	const BOLLINGER_CONDITION Bollinger_cond;

	/** ENTRY: */
	bool denied_divergence_general(DivergenceType divType, StatType statType, const int max_neg_period, const RSI_condition RSI_cond = LSTAT_LBAR, bool no_open = false);
	bool denied_divergence(DivergenceType divType, const int max_neg_period, const RSI_condition RSI_cond);
	bool double_divergence(DivergenceType divType, /* StatType statType, */ bool no_open = false);
	/** EXIT: */
	void opposite_divergence(DivergenceType divType, MTrade* curr_trade);
	void bollinger_crossing(MTrade* curr_trade, const BOLLINGER_CONDITION Bollinger_cond = OUTER_BANDS);
	void negative_trade_expiration(MTrade* curr_trade);
	void stop_loss_take_profit(MTrade* curr_trade, const double close);
	void check_entry_conditions(DivergenceType divType, const int max_neg_period, const RSI_condition RSI_cond);
    void check_exit_conditions(MTrade* curr_trade, DivergenceType divType, const BOLLINGER_CONDITION Bollinger_cond);
	const double stop_loss;
	const double take_profit;
	const int    expirationBars;

	/** TRADES: (live & backtesting) */
	const double default_order_size;
	double pl = 0.0;
    double plArray [MAXBARS];
	void update_PnL();

	bool is_trade_initialized(MTrade * trade);
	bool is_trade_open(MTrade * trade);
	double trade_opening_price(MTrade * trade);
	double trade_closing_price(MTrade * trade);
	int get_trade_bars(MTrade * trade);
	double trade_opening_order_size(MTrade * trade);
	double trade_balance(MTrade * trade);

	void general_open(const TradeDirection dir, 
					  const int bar_index, 
					  std::string orderRef   = "",
					  const double orderSize = -1.0);
	void general_close(MTrade * trade, const std::string orderRef = "");
	void close_trades(const TradeDirection direction_to_close, std::string orderRef);

	/** DATAOUTPUT: */ 

	char* reportPath;
    char* logPath;

	static bool open_path_or_stdout(FILE * &fp, const char * path);
	std::string print_trade(MTrade * trade, int trade_no);

	public:
	void print_indicators(const std::string outputDir, const std::string outputExt = ".txt");
	void print_bars(const std::string outputDir, const std::string outputExt = ".txt");
	void print_PL_data(const std::string outputDir, const std::string outputExt = ".txt");
	void print_backtest_results();

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
			// Grandezza ordini
				const double def_order_size = 1.0,
			// Parameters for denied divergence
				const DivergenceType divType = LONG_DIV, 
				const int max_neg_period = 14, 
				const RSI_condition RSI_cond = LSTAT_LBAR,
				const BOLLINGER_CONDITION Bollinger_cond = OUTER_BANDS,
			// Backtesting results directories
				const std::string bt_report_dir = "../backtesting/reports/", 
				const std::string bt_log_dir    = "../backtesting/logs/",
				const std::string file_ext      = ".txt"
				);
	~Strategy();

	/* Interface to communicate with MClient */
	const std::string strategy_code;
	TradeData * m_tradeData;
	MTrade * trades2open  [MAXOPENTRADES];
	MTrade * trades2close [MAXCLOSETRADES];
	int opening_trades = 0;
	int closing_trades = 0;
	
	int get_instr_id();
	void set_trading_state(TradingState t_state);
	TradingState get_trading_state();
	void handle_realTimeBar(const double close);
	void handle_barUpdate();
	std::string strat_info();

	
};

