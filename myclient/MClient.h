
#pragma once

#include <ctype.h>

#include "StdAfx.h"
#include "EWrapper.h"
#include "EReaderOSSignal.h"
#include "EClientSocket.h"
#include "EReader.h"
#include "Order.h"
#include "TagValue.h"
#include "CommissionReport.h"
#include "Execution.h"

#include "Mlogger.h"
#include "Instrument.h"
#include "MOrders.h"

enum State {
    ST_CONNECT,
    REQCONTRACTDETAILS,
    REQCONTRACTDETAILS_ACK,
    REQHISTORICALDATA,
    REQHISTORICALDATA_ACK,
    REQIDS,
    REQIDS_ACK,
    PLACEORDER,
    PLACEORDER_ACK,
    CANCELORDER,
    CANCELORDER_ACK,
    REQALLOPENORDERS,
    REQALLOPENORDERS_ACK,
    REQREALTIMEBARS,
    REQREALTIMEBARS_ACK
};

class MClient : public EWrapper
{
    //** States & requests **// 
    State     m_state;
    int       req_timeout = 10;  // maximum seconds waited before giving up on a request
    
    //** Logger **//
    Mlogger* const m_logger;

    //** Instruments **//
    int            m_instr_Id = 0;  // current number of instruments
    const int      maxInstr;
    Instrument**   m_instrArray;

    //** Orders **//
    OrderId   m_orderId;
    OrderId   m_cancel_orderId; // Id of order requesting to cancel
    bool      m_extraAuth;
    const int maxOrd;           // Maximum number of orders held within client
    int       num_ord = 0;      // Number of orders currently held within client
    Order**   m_ordArray;       // Array of order structs (to keep track of all currently open orders)

    //** ClientSocket server connection **//
    EClientSocket* const m_Client;
    EReaderOSSignal      m_osSignal;
    EReader*             m_Reader;

    /**** PRIVATE REQUEST METHODS ***/
    
    bool waitResponse(State target, int reqId, std::string caller);

    bool reqIds	(int numIds = -1); 

    bool reqContractDetails	(int reqId, Contract contract);

    bool reqHistoricalData(	int 	         tickerId,
                            Contract 	     contract,
                            string 	         endDateTime,
                            string 	         durationStr,
                            string 	         barSizeSetting,
                            string 	         whatToShow,
                            int 	         useRTH,
                            int 	         formatDate,
                            bool 	         keepUpToDate,
                            std::shared_ptr<TagValueList> chartOptions = TagValueListSPtr()
    );

    bool reqRealTimeBars( int 	    tickerId,
                          Contract 	contract,
                          int 	    barSize,
                          string 	whatToShow,
                          bool 	    useRTH,
                          std::shared_ptr<TagValueList> realTimeBarsOptions = TagValueListSPtr()
    );

    


    //** STILL TO IMPLEMENT **///
    
    void cancelRealTimeBars	(int tickerId);		



    /**** PUBLIC METHODS ***/
    public:

    bool reqAllOpenOrders();

    #include "EWrapper_prototypes.h"

    MClient( const std::string logPath = "./MClient_log.txt", 
             const int maxInstr = 128, const int maxOrd = 10000);
    ~MClient();

    /*** CONNECTIVITY ***/
    bool connect(const char * host, int port, int clientId = 0);
	void disconnect() const;
	bool isConnected() const;

    /*** HANDLING INSTRUMENTS ***/
    void add_Instrument( const std::string barSize, ContractDetails dataContract, 
        ContractDetails orderContract, Instrument::ReqIds reqIds, std::string logPath);

    /*** Get instrument with any matching reqId ***/
    /*** Important: all reqIds must be different across Instruments ****/
    Instrument* reqId_Instrument( int reqId);

    /*** Get instrument from its number ***/
    Instrument* get_Instrument( int instr_Id);
    
    /**** Update each intrument's contracts ***/
    void update_contracts();

    /*** Initialize each instrument's bars at start-up ***/
    void initialize_bars(std::string timePeriod, std::string whatToShow = "TRADES", int useRTH = 1);

    /*** Update each instrument's bars (at realTimeBar callback) ***/
    void update_bars(std::string whatToShow = "TRADES", int useRTH = 1, int factor = 2);

    /*** ORDERS ***/
    Order* get_Order( int orderId);
    void update_orders(); // retrieve from server all orders currently open
    int placeOrder( int inst_id, Order order);
    bool cancelOrder( int orderId);

    //*** HELPERS / UTILS ***//
    static char* currTime_str();
    char* extend_dur(std::string barSize, int factor = 2);


};

