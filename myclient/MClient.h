
#pragma once

#include <ctype.h>
#include <fstream>

#include "StdAfx.h"
#include "EWrapper.h"
#include "EReaderOSSignal.h"
#include "EClientSocket.h"
#include "EReader.h"
#include "Order.h"
#include "TagValue.h"
#include "CommissionReport.h"
#include "Execution.h"
#include "MContractDetails.h"

#include "Mlogger.h"
#include "Instrument.h"
#include "MOrders.h"
#include "MTrade.h"
#include "Strategy.h"
#include "CommonMacros.h"
#include "CommonEnums.h"
#include "ser.h"
#include "memdbg.h"

class MClient : public EWrapper
{
    public:
    //** States & requests **// 
    State m_state;
    int   req_timeout = 20;  // maximum seconds waited before giving up on a request
    
    //** Logger **//
    Mlogger* const m_logger;

    //** Instruments & Strategies **//
    int         m_instr_Id = 0;  // current number of instruments
    Instrument* m_instrArray[MAXINSTR];
    Strategy*   m_stratArray[MAXINSTR][MAXSTRAT]; // Array mapping each instrument to its set of strategies
    int         m_stratCount[MAXINSTR] = {0};    // Array keeping track of how many strategies each instrument holds

    //** Orders **//
    OrderId   m_orderId;
    OrderId   m_cancel_orderId; // Id of order requesting to cancel
    Order *   rec_order; // most recent order
    bool      m_extraAuth;

    //** ClientSocket server connection **//
    EReaderOSSignal      m_osSignal;
    EClientSocket* const m_Client;
    EReader*             m_Reader;

    //** Trades & serialization **//
    TradeData * m_tradeData;
    const std::string serFileName  = "../ser/tradeData.txt";
    void archive_td_out();
    void archive_td_in();

    /**** PRIVATE ECLIENT REQUEST METHODS (wrapping around EClient functions) ***/
    bool waitResponse(State target, int reqId, std::string caller);
    bool reqIds	(int numIds = -1); 
    bool reqContractDetails	(int reqId, Contract contract);
    bool reqHistoricalData(	int 	         tickerId,
                            Contract 	     contract,
                            std::string 	 endDateTime,
                            std::string 	 durationStr,
                            std::string 	 barSizeSetting,
                            std::string 	 whatToShow,
                            int 	         useRTH,
                            int 	         formatDate,
                            bool 	         keepUpToDate,
                            std::shared_ptr<TagValueList> chartOptions = TagValueListSPtr()
    );
    bool reqRealTimeBars( int 	      tickerId,
                          Contract    contract,
                          int 	      barSize,
                          std::string whatToShow,
                          bool 	      useRTH,
                          std::shared_ptr<TagValueList> realTimeBarsOptions = TagValueListSPtr()
    );
    bool reqExecutions( ExecutionFilter execFilter = ExecutionFilter());
    bool reqCompletedOrders( bool apiOnly = false);
    bool reqAllOpenOrders();
    void reqAutoOpenOrders( bool autoBind = true);
    bool reqPositions();
    void reqGlobalCancel();
    void testSerFile();

    //** STILL TO IMPLEMENT **///
    void cancelRealTimeBars	(int tickerId);		

    // public:

    #include "EWrapper_prototypes.h"

    //** Request Ids **// 
    typedef struct RI {
        int reqExecutions;
        RI( const int reqExecutions)
            : reqExecutions(reqExecutions)
        {
        }
    } ReqIds;
    ReqIds m_reqIds;

    MClient( const std::string logPath = "./MClient_log.txt", 
             ReqIds reqIds             = ReqIds(17001),
             bool init_trade_data      = false);
    ~MClient();

    /*** CONNECTIVITY ***/
    bool connect(const char * host, int port, int clientId = 0);
	void disconnect() const;
	bool isConnected() const;

    /*** HANDLING INSTRUMENTS ***/
    int add_Instrument(const std::string barSize, 
                        ContractDetails dataContract, 
                        ContractDetails orderContract, 
                        Instrument::ReqIds reqIds, 
                        std::string logPath
    );
    Instrument* get_Instrument(int instr_Id);
    Instrument* reqId_Instrument(int reqId);  /** IMPORTANT: all reqIds must be different across Instruments */
    void update_contracts();

    /*** BARS ***/
    void update_instr_bars(int instr_id, int numBars, bool initialization, std::string whatToShow = "TRADES", int useRTH = 1);
    void update_bars(int numBars, bool initialization, std::string whatToShow = "TRADES", int useRTH = 1);
    static char* currTime_str();
    
    //** Orders & Trades **//
    void add_Strategy(const int instr_id, Strategy * strategy);
    int placeOrder(int inst_id, Order order);
    bool cancelOrder(int orderId);
    int openTrade  (Strategy * strategy, const int trade_arr_pos);
    bool closeTrade(Strategy * strategy, const int trade_arr_pos);
    void close_strat_trades(Strategy * strategy);
    void open_strat_trades(Strategy * strategy);

    /** DATAOUTPUT: **/
    private:
    void print_indicators(Strategy * const strat, const std::string outputDir, const std::string outputExt = ".txt");
    void print_bars(Strategy * const strat, const std::string outputDir, const std::string outputExt = ".txt");
    void print_PL_data(Strategy * const strat, const std::string outputDir, const std::string outputExt = ".txt");
    void print_backtest_results(Strategy * const strat);
    void set_trading_state(Strategy * const strat, TradingState t_state);
    Strategy * find_strategy(std::string strategy_code);

    void strategy_iterate( void (MClient::* data_out_func)(Strategy * const, const std::string, const std::string) = NULL, 
										const std::string outputDir = "", const std::string outputExt = ".txt",
							void (MClient::* t_state_func)(Strategy * const, const TradingState) = NULL, const TradingState t_state = RETRIEVAL,
							void (MClient::* no_arg_func) (Strategy * const) = NULL);

    public:
    void print_indicators(const std::string outputDir, const std::string outputExt = ".txt");
    void print_bars(const std::string outputDir, const std::string outputExt = ".txt");
    void print_PL_data(const std::string outputDir, const std::string outputExt = ".txt");
    void print_backtest_results();
    void set_trading_state(TradingState t_state);

    /*** EWRAPPER CALLBACK HANDLERS ***/
    private:

    void handle_currentTime( long time);
    void handle_error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson);
    void handle_nextValidId( OrderId orderId);
    void handle_contractDetails( int reqId, const ContractDetails& contractDetails);
    void handle_contractDetailsEnd( int reqId);
    void handle_historicalDataUpdate(TickerId reqId, const Bar& bar);
    void handle_historicalData(TickerId reqId, const Bar& bar);
    void handle_historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr);
    void handle_realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                            Decimal volume, Decimal wap, int count);
    void handle_openOrder( OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState);
    void handle_orderStatus(OrderId orderId, const std::string& status, Decimal filled,
        Decimal remaining, double avgFillPrice, int permId, int parentId,
        double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice);
    void handle_openOrderEnd();
    void handle_completedOrder(const Contract& contract, const Order& order, const OrderState& orderState);
    void handle_completedOrdersEnd();
    void handle_position( const std::string& account, const Contract& contract, Decimal position, double avgCost);
    void handle_positionEnd();
    void handle_execDetails(int reqId, const Contract& contract, const Execution& execution);
    void handle_execDetailsEnd(int reqId);

};

