
#pragma once 

#include "BackTester.h"
#include "Indicators.h"
#include "MTrade.h"
#include "MOrders.h"
#include "memdbg.h"

#define MAXENTRYCONDS 8
#define MAXEXITCONDS  8

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

class Strategy
{
	/** INSTRUMENT: */
	Instrument * /*const*/ m_instr;  
	int curr_bar_index = 0; // bar iteration

	/** INDICATORS: */
	// Indicators required for each strategy
	Indicators::LocalMin*       m_LocalMin;
	Indicators::LocalMax*       m_LocalMax;
	Indicators::RSI*            m_RSI;
	Indicators::LongDivergence* m_LongDivergence;
	Indicators::BollingerBands* m_BollingerBands;
	void compute_indicators();
	void delete_indicators();

	/** CONDITIONS: */
	EntryConditions entry_conditions [MAXENTRYCONDS];
    ExitConditions  exit_conditions  [MAXEXITCONDS];
	/** ENTRY: */
	void denied_divergence_local_max(); // Helper
	void denied_divergence_local_min(); // Helper
	void denied_divergence();
	void double_divergence();
	/** EXIT: */
	void opposite_divergence(MTrade_t* curr_trade = NULL);
	void bollinger_crossing(MTrade_t* curr_trade  = NULL);
	void negative_trade_expiration(MTrade_t* curr_trade = NULL);
	void stop_loss_take_profit(MTrade_t* curr_trade = NULL, const double close = -DBL_MAX);
	void check_entry_conditions();
    void check_exit_conditions(MTrade_t* curr_trade = NULL);
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
				Indicators::LongDivergence * input_LongDivergence,
			// Condition arrays (to be copied into instance variables)
				EntryConditions input_entry_conditions [],
				ExitConditions  input_exit_conditions  []
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

