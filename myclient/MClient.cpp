
#include "MClient.h"

MClient::MClient( const std::string logPath, const int maxInstr, const int maxOrd) :
      m_osSignal(2000)	// 2-seconds timeout
	, m_orderId(0)
    , m_extraAuth(false)
	, m_logger(new Mlogger( logPath))
	, maxInstr(maxInstr)		// maximum instruments
	, maxOrd(maxOrd)			// maximum orders
	, m_state(ST_CONNECT)		// initialize state
	, m_Client(new EClientSocket(this, &m_osSignal)) // Finally pass instance to EClientSocket
{
	// Initialize Instrument & Order arrays
	this->m_instrArray = new Instrument*[maxInstr];
	this->m_ordArray   = new Order*[maxOrd];
}

MClient::~MClient()
{
	// Clean up Instrument & Order arrays
	for (int i = 0; i < m_instr_Id; i++) {
		if (m_instrArray[i] != NULL) {
			delete(m_instrArray[i]);
		}
	}
	for (int i = 0; i < num_ord; i++) {
		if (m_ordArray[i] != NULL) {
			delete(m_ordArray[i]);
		}
	}
	delete m_Reader;
	delete m_Client;
	delete m_logger;
}

/*** CONNECTIVITY ***/

bool MClient::connect(const char *host, int port, int clientId)
{
	printf( "Connecting to %s:%d clientId:%d\n", !( host && *host) ? "127.0.0.1" : host, port, clientId);
	bool cRes = m_Client->eConnect( host, port, clientId, m_extraAuth);
	if (cRes) {
		printf( "Connected to %s:%d clientId:%d\n", m_Client->host().c_str(), m_Client->port(), clientId);
		m_Reader = new EReader(m_Client, &m_osSignal);
		m_Reader->start();
	} else {
		printf( "Cannot connect to %s:%d clientId:%d\n", m_Client->host().c_str(), m_Client->port(), clientId);
    }
	return cRes;
}

void MClient::connectAck() {
	if (!m_extraAuth && m_Client->asyncEConnect()) {
		m_Client->startApi();
		printf ( "Started API\n");
	}
}

void MClient::disconnect() const {
	m_Client->eDisconnect();
	printf ( "Disconnected\n");
}

bool MClient::isConnected() const {
	return m_Client->isConnected();
}


/**** PRIVATE REQUEST METHODS ****/

bool MClient::waitResponse( State target, int reqId, std::string caller) {
	// Get current system time
	time_t start_time = time(NULL);
	// Iterate until time runs out
	while (	time(NULL) - start_time < req_timeout) {
		// If goal state reached, return
		if ( m_state == target) {
			m_logger->str("\t Id " + to_string(reqId) + " from " + caller 
							+ " returned successfully\n");
			return true;
		}
		// Keep quering other thread
		m_osSignal.waitForSignal();
		errno = 0;
		m_Reader->processMsgs();
	}
	m_logger->str("\t Id " + to_string(reqId) + " from " + caller 
					+ " timed out\n");
	return false;
}

bool MClient::reqIds(int numIds) {
	// SET REQUEST STATE
	m_state = REQIDS;
	// Make request
	m_Client->reqIds(numIds);
	// Wait for response, and pass return value
	return waitResponse(REQIDS_ACK, numIds, "reqIds()");
}

bool MClient::reqContractDetails(int reqId, Contract contract) {
	// SET REQUEST STATE
	m_state = REQCONTRACTDETAILS;
	// Make a request
	m_Client->reqContractDetails( reqId, contract);
	// Wait for response, and pass return value
	return waitResponse(REQCONTRACTDETAILS_ACK, reqId, "reqContractDetails()");
}

bool MClient::reqHistoricalData( int tickerId, Contract contract, string endDateTime,
								string durationStr, string barSizeSetting, string whatToShow,
						        int useRTH, int formatDate, bool keepUpToDate,
								std::shared_ptr<TagValueList> chartOptions) {
	// SET REQUEST STATE
	m_state = REQHISTORICALDATA;
	// Make a request
	m_Client->reqHistoricalData( tickerId, contract, endDateTime, durationStr, barSizeSetting, whatToShow, useRTH, formatDate, keepUpToDate, chartOptions);
	// Wait for response, and pass return value
	return waitResponse(REQHISTORICALDATA_ACK, tickerId, "reqHistoricalData()");
}

bool MClient::reqAllOpenOrders() {
	// Set request state
	m_state = REQALLOPENORDERS;
	// Make the request
	m_Client->reqAllOpenOrders();
	// Wait response
	return waitResponse(REQALLOPENORDERS_ACK, -1, "reqAllOpenOrders()");
}

bool MClient::reqRealTimeBars( int 	    tickerId,
							   Contract contract,
							   int 	    barSize,
							   string 	whatToShow,
							   bool 	useRTH,
						       std::shared_ptr<TagValueList> realTimeBarsOptions) {
	// Set request state
	m_state = REQREALTIMEBARS;
	// Make the request
	m_Client->reqRealTimeBars( tickerId, contract, barSize, whatToShow, useRTH, realTimeBarsOptions);
	// Wait response
	return waitResponse( REQREALTIMEBARS_ACK, tickerId, "reqRealTimeBars()");
}


/*** STILL TO IMPLEMENT ****/

void MClient::cancelRealTimeBars(int tickerId) {
	m_Client->cancelRealTimeBars(tickerId);
}


/*** ORDERS ***/

Order* MClient::get_Order(int orderId) {
	for (int i = 0; i < num_ord; i++) {
		if (m_ordArray[i]->orderId == orderId) {
			return m_ordArray[i];
		}
	}
	return NULL;
}

void MClient::update_orders() {

}

int MClient::placeOrder( int inst_id, Order order) {
	// Attemp to retrieve the given instrument
	Instrument* instr = get_Instrument(inst_id);
	if ( instr == NULL ) {
		m_logger->str("\t\n Cannot place order: invalid inst_id\n\n");
		return -1;
	}
	// Request and update next valid orderId
	if (!reqIds()) {
		m_logger->str("\t\n Cannot place order: error receiving next valid orderId\n\n");
		return -1;
	}
	// Set request state
	m_state = PLACEORDER;
	// Make the request
	m_Client->placeOrder( m_orderId, instr->orderContract.contract, order);
	// Wait reception
	if ( !waitResponse( PLACEORDER_ACK, m_orderId, "placeOrder()")) {
		return -1;
	}
	// Return orderId upon successful execution
	return m_orderId;
}

bool MClient::cancelOrder( int orderId) {
	// Set request state and order-to-cancelId
	m_state = CANCELORDER;
	m_cancel_orderId = orderId;
	// Make the request
	m_Client->cancelOrder(orderId, "");
	// Wait for response
	return waitResponse( CANCELORDER_ACK, orderId, "cancelOrder()");
}

/*** HANDLING INSTRUMENTS ***/

void MClient::add_Instrument( const std::string barSize, ContractDetails dataContract, 
        ContractDetails orderContract, Instrument::ReqIds reqIds, std::string logPath) {
	if ( m_instr_Id < maxInstr) {
		// Add new instrument to array
		m_instrArray[m_instr_Id] = new Instrument( m_instr_Id, barSize, dataContract,
												orderContract, reqIds, logPath);
		// Increment current instrument Id
		m_instr_Id++;
	} else {
		m_logger->str("\t Cannot add instrument: maximum number of instruments reached.\n");
	}
}

Instrument* MClient::get_Instrument( int instr_Id) {
	if ( instr_Id < m_instr_Id) {
		return m_instrArray[instr_Id];
	}
	m_logger->str("\n\t ERROR: Instrument " + to_string(instr_Id) + " out of range\n\n");
	return NULL;
}

void MClient::update_contracts() {
	for (int i = 0; i < m_instr_Id; i++) {
		// For each instrument, request contract details and handle response
		Instrument* instr = m_instrArray[i];
		reqContractDetails( instr->m_reqIds.dataContract, instr->dataContract.contract);
		reqContractDetails( instr->m_reqIds.orderContract, instr->orderContract.contract);
	}
}

Instrument* MClient::reqId_Instrument( int reqId) {
	for (int i = 0; i < m_instr_Id; i++) {
		if ( m_instrArray[i]->m_reqIds.dataContract   == reqId
		  || m_instrArray[i]->m_reqIds.orderContract  == reqId
		  || m_instrArray[i]->m_reqIds.realTimeBars   == reqId
		  || m_instrArray[i]->m_reqIds.historicalBars == (TickerId) reqId
		  || m_instrArray[i]->m_reqIds.updatedBars    == (TickerId) reqId) {
			return m_instrArray[i];
		}
	}
	return NULL;
}

void MClient::initialize_bars(std::string timePeriod, std::string whatToShow, int useRTH) {
	// Iterate through all instruments
	for (int i = 0; i < m_instr_Id; i++) {
		// For each instrument, first request historical bars details until current time
		Instrument* instr = m_instrArray[i];
		// When bars are initialized, keepUpToDate = false so that we can confirm reception
		char* queryTime = currTime_str();
		if (!reqHistoricalData( instr->m_reqIds.historicalBars, instr->dataContract.contract, queryTime, 
					timePeriod, instr->barSize, whatToShow, useRTH, 1, false)) {
			m_logger->str("\tError retrieving historical data for instrument " + to_string(i) + "\n");
		}
		// Free string(s)
		delete(queryTime);
	}
}

void MClient::update_bars( std::string whatToShow, int useRTH, int factor) {
	// Iterate through all instruments
	for (int i = 0; i < m_instr_Id; i++) {
		Instrument* instr = m_instrArray[i];
		// Determine if instrument requires an update
		if ( (int)(instr->curr_sys_time() - instr->last_bar_update)
				>= instr->sec_barSize ) 
		{
			// Extend duration string to make request
			char* durStr = extend_dur(instr->barSize, factor);
			// Handle exception
			if ( durStr == NULL) {
				m_logger->str( "\tupdate_bars(): extend_dur() error for instrument "
								+ to_string(i) + "\n");
				continue;
			}
			// Make the request
			char* queryTime = currTime_str();
			if ( !reqHistoricalData( instr->m_reqIds.updatedBars, instr->dataContract.contract, queryTime,
							 durStr, instr->barSize, whatToShow, useRTH, 1, false) ) {
				m_logger->str("\tError updating historical data for instrument " + to_string(i) + "\n");
			}
			// Free strings
			delete(queryTime);
			delete(durStr);
		}
	}
}

/*** MALLOC'D STRING MUST BE FREED ***/
char* MClient::extend_dur( std::string barSize, int factor)
{
	// Tokenize string
	const int size = barSize.size();
	char cstr[size + 1];
	int dur;
  	if ( sscanf( barSize.c_str(), "%d %s", &dur, cstr) != 2) {
		m_logger->str( "\textend_dur() error: invalid barSize string\n");
		return NULL;
	}	
	// Generate output string
	int  tf = 1;
	char tc = 'S';
	switch ( char t_unit = cstr[0]) {
		case 's': 				 break;					   
		case 'm': tf = 60;	     break;
		case 'h': tf = 60 * 60;  break;
		case 'd': tc = 'D'; 	 break;
		default:
			m_logger->str( "\textend_dur() error: unsupported barSize\n");
			return NULL;
	}
	// Construct output string
	const int out_len = 8;
	char buff[out_len];
	int char_written = snprintf( buff, out_len, "%d %c", 
						(tf * factor * dur), tc);
	m_logger->str( "\textend_dur(): snprintf() recorded " + to_string(char_written) + " characters\n");
	// Handle exception
	if (char_written < 0) {
		m_logger->str( "\textend_dur(): snprintf() failed\n");
		return NULL;
	}
	// Allocate the correct amount of memory to fully free string later
	char* out = new char[char_written + 1]; // include terminating null character
	strncpy( out, buff,  char_written + 1);
	// Log output
	m_logger->str( "\textend_dur(" + barSize + ") = " + string(out) + "\n");
	return out;	
}

/*** MALLOC'D STRING MUST BE FREE'D ***/
char* MClient::currTime_str() {
	// Get current time
	std::time_t rawtime;
    std::tm* timeinfo;
    char* queryTime = new char[80];
	std::time(&rawtime);
    timeinfo = std::gmtime(&rawtime);
	std::strftime(queryTime, 80, "%Y%m%d-%H:%M:%S", timeinfo);
	return queryTime;
}


/******** EWRAPPER MAIN CALLBACKS *******/

void MClient::currentTime( long time) {
	m_logger->currentTime(time);
}

void MClient::error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson) {
	m_logger->error(id, errorCode, errorString, advancedOrderRejectJson);
}

void MClient::connectionClosed() {
	printf( "Connection Closed\n");
}

void MClient::nextValidId( OrderId orderId) {
	// ACKNOWLEDGE RECEIVAL OF NEXT VALID ID, and record next valid order Id
	m_state = REQIDS_ACK;
	m_logger->str( "Next Valid Id: " + std::to_string(orderId) + "\n");
	m_orderId = orderId;
}

void MClient::contractDetails( int reqId, const ContractDetails& contractDetails) {
	// Log call
	m_logger->contractDetails(reqId, contractDetails);
	// Identify Instrument corresponding to given reqId
	Instrument* instr = reqId_Instrument(reqId);
	if ( instr == NULL) {
		m_logger->str("\n\t ERROR: conractDetails reqId not found\n\n");
		return;
	}
	// Update the instrument's Contracts
	instr->updateDataContract (reqId, contractDetails);
	instr->updateOrderContract(reqId, contractDetails);
}

void MClient::contractDetailsEnd( int reqId) {
	// ACKNOWLEDGE RECEIVAL OF REQCONTRACTDETAILS REQUEST
	this->m_state = REQCONTRACTDETAILS_ACK;
	m_logger->str( "ContractDetailsEnd: reqId: " + std::to_string(reqId) + "\n");
}

void MClient::historicalDataUpdate(TickerId reqId, const Bar& bar) {
	// NEED TO PROTECT INSTRUMENT FROM INCOMPLETE BARS
    m_logger->historicalData(reqId, bar);
}

void MClient::historicalData(TickerId reqId, const Bar& bar) {
	// Log call
    m_logger->historicalData(reqId, bar);
	// Identify Instrument corresponding to given reqId
	Instrument* instr = reqId_Instrument((int)reqId);
	if ( instr == NULL) {
		m_logger->str("\n\t ERROR: historicalDataUpdate reqId not found\n\n");
		return;
	}
	// Try to update the instrument's Bars
	instr->addBar(reqId, bar);
}

void MClient::historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) {
	// ACKNOWLEDGE RECEIVAL OF REQCONTRACTDETAILS REQUEST
	this->m_state = REQHISTORICALDATA_ACK;
	m_logger->str( "HistoricalDataEnd. ReqId: " + std::to_string(reqId) + " - Start Date: " + startDateStr + ", End Date: " + endDateStr + "\n" );
}

void MClient::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                                Decimal volume, Decimal wap, int count) {
	// Log call
    m_logger->realtimeBar( reqId, time, open, high, low, close, volume, wap, count);
	// Acknowledge receival of FIRST BAR only
	if (m_state == REQREALTIMEBARS) {
		m_state =  REQREALTIMEBARS_ACK;
	}
}

void MClient::openOrder( OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
	// Log call
	m_logger->openOrder( orderId, contract, order, orderState);
	// Mark successful order execution only if orderId corresponds with the current one & order recently placed
	if ( m_state == PLACEORDER && m_orderId == orderId) {
		m_state = PLACEORDER_ACK;
	}
	// Handle open orders information receival
	if ( m_state == REQALLOPENORDERS) {
		printf( "\tReceived orderId %ld, orderRef %s\n", order.orderId, order.orderRef.c_str());

		//*** FORWARD ORDER INFORMATION TO STRATEGY ***//
	}
}

void MClient::orderStatus(OrderId orderId, const std::string& status, Decimal filled,
		Decimal remaining, double avgFillPrice, int permId, int parentId,
		double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) {
	// Log call
    m_logger->orderStatus(orderId, status, filled, remaining, avgFillPrice, permId, parentId, lastFillPrice, clientId, whyHeld, mktCapPrice);
	// Confirm order cancellation
	if ( m_state == CANCELORDER && m_cancel_orderId == orderId &&
		( status == "PendingCancel"
	   || status == "ApiCancelled"
	   || status == "Cancelled")) 
	{
		m_state = CANCELORDER_ACK;
	}
}

void MClient::openOrderEnd() {
	m_logger->str( "OpenOrderEnd\n");
	// Confirm receival of all open orders
	m_state = REQALLOPENORDERS_ACK;
}



/******** EWRAPPER CALLBACKS STILL TO BE USED *******/



void MClient::completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {
    m_logger->completedOrder( contract, order, orderState);

}

void MClient::completedOrdersEnd() {
	m_logger->str( "CompletedOrdersEnd\n");

}



/******** EWRAPPER UNUSED CALLBACKS (CURRENTLY LOGGED TO STDOUT) *******/

void MClient::tickPrice( TickerId tickerId, TickType field, double price, const TickAttrib& attribs) {
    printf( "Tick Price. Ticker Id: %ld, Field: %d, Price: %s, CanAutoExecute: %d, PastLimit: %d, PreOpen: %d\n", tickerId, (int)field, Utils::doubleMaxString(price).c_str(), attribs.canAutoExecute, attribs.pastLimit, attribs.preOpen);
}

void MClient::tickSize( TickerId tickerId, TickType field, Decimal size) {
	printf( "Tick Size. Ticker Id: %ld, Field: %d, Size: %s\n", tickerId, (int)field, decimalStringToDisplay(size).c_str());
}

void MClient::tickOptionComputation( TickerId tickerId, TickType tickType, int tickAttrib, double impliedVol, double delta,
                                          double optPrice, double pvDividend,
                                          double gamma, double vega, double theta, double undPrice) {
    printf( "TickOptionComputation. Ticker Id: %ld, Type: %d, TickAttrib: %s, ImpliedVolatility: %s, Delta: %s, OptionPrice: %s, pvDividend: %s, Gamma: %s, Vega: %s, Theta: %s, Underlying Price: %s\n", 
        tickerId, (int)tickType, Utils::intMaxString(tickAttrib).c_str(), Utils::doubleMaxString(impliedVol).c_str(), Utils::doubleMaxString(delta).c_str(), Utils::doubleMaxString(optPrice).c_str(), 
        Utils::doubleMaxString(pvDividend).c_str(), Utils::doubleMaxString(gamma).c_str(), Utils::doubleMaxString(vega).c_str(), Utils::doubleMaxString(theta).c_str(), Utils::doubleMaxString(undPrice).c_str());
}

void MClient::tickGeneric(TickerId tickerId, TickType tickType, double value) {
    printf( "Tick Generic. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, Utils::doubleMaxString(value).c_str());
}

void MClient::tickString(TickerId tickerId, TickType tickType, const std::string& value) {
	printf( "Tick String. Ticker Id: %ld, Type: %d, Value: %s\n", tickerId, (int)tickType, value.c_str());
}

void MClient::tickEFP(TickerId tickerId, TickType tickType, double basisPoints, const std::string& formattedBasisPoints,
                            double totalDividends, int holdDays, const std::string& futureLastTradeDate, double dividendImpact, double dividendsToLastTradeDate) {
    printf( "TickEFP. %ld, Type: %d, BasisPoints: %s, FormattedBasisPoints: %s, Total Dividends: %s, HoldDays: %s, Future Last Trade Date: %s, Dividend Impact: %s, Dividends To Last Trade Date: %s\n", 
        tickerId, (int)tickType, Utils::doubleMaxString(basisPoints).c_str(), formattedBasisPoints.c_str(), Utils::doubleMaxString(totalDividends).c_str(), Utils::intMaxString(holdDays).c_str(), 
        futureLastTradeDate.c_str(), Utils::doubleMaxString(dividendImpact).c_str(), Utils::doubleMaxString(dividendsToLastTradeDate).c_str());
}

void MClient::winError( const std::string& str, int lastError) {}

void MClient::updateAccountValue(const std::string& key, const std::string& val,
                                       const std::string& currency, const std::string& accountName) {
	printf("UpdateAccountValue. Key: %s, Value: %s, Currency: %s, Account Name: %s\n", key.c_str(), val.c_str(), currency.c_str(), accountName.c_str());
}

void MClient::updatePortfolio(const Contract& contract, Decimal position,
                                    double marketPrice, double marketValue, double averageCost,
                                    double unrealizedPNL, double realizedPNL, const std::string& accountName){
    printf("UpdatePortfolio. %s, %s @ %s: Position: %s, MarketPrice: %s, MarketValue: %s, AverageCost: %s, UnrealizedPNL: %s, RealizedPNL: %s, AccountName: %s\n", 
        (contract.symbol).c_str(), (contract.secType).c_str(), (contract.primaryExchange).c_str(), decimalStringToDisplay(position).c_str(), 
        Utils::doubleMaxString(marketPrice).c_str(), Utils::doubleMaxString(marketValue).c_str(), Utils::doubleMaxString(averageCost).c_str(), 
        Utils::doubleMaxString(unrealizedPNL).c_str(), Utils::doubleMaxString(realizedPNL).c_str(), accountName.c_str());
}

void MClient::updateAccountTime(const std::string& timeStamp) {
	printf( "UpdateAccountTime. Time: %s\n", timeStamp.c_str());
}

void MClient::accountDownloadEnd(const std::string& accountName) {
	printf( "Account download finished: %s\n", accountName.c_str());
}

void MClient::bondContractDetails( int reqId, const ContractDetails& contractDetails) {
	printf( "BondContractDetails begin. ReqId: %d\n", reqId);
	m_logger->printBondContractDetailsMsg(contractDetails);
	printf( "BondContractDetails end. ReqId: %d\n", reqId);
}

void MClient::execDetails( int reqId, const Contract& contract, const Execution& execution) {
	printf( "ExecDetails. ReqId: %d - %s, %s, %s - %s, %s, %s, %s, %s\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), Utils::longMaxString(execution.orderId).c_str(), decimalStringToDisplay(execution.shares).c_str(), decimalStringToDisplay(execution.cumQty).c_str(), Utils::intMaxString(execution.lastLiquidity).c_str());
}

void MClient::execDetailsEnd( int reqId) {
	printf( "ExecDetailsEnd. %d\n", reqId);
}

void MClient::updateMktDepth(TickerId id, int position, int operation, int side,
                                   double price, Decimal size) {
    printf( "UpdateMarketDepth. %ld - Position: %s, Operation: %d, Side: %d, Price: %s, Size: %s\n", id, Utils::intMaxString(position).c_str(), operation, side, 
        Utils::doubleMaxString(price).c_str(), decimalStringToDisplay(size).c_str());
}

void MClient::updateMktDepthL2(TickerId id, int position, const std::string& marketMaker, int operation,
                                     int side, double price, Decimal size, bool isSmartDepth) {
    printf( "UpdateMarketDepthL2. %ld - Position: %s, Operation: %d, Side: %d, Price: %s, Size: %s, isSmartDepth: %d\n", id, Utils::intMaxString(position).c_str(), operation, side, 
        Utils::doubleMaxString(price).c_str(), decimalStringToDisplay(size).c_str(), isSmartDepth);
}

void MClient::updateNewsBulletin(int msgId, int msgType, const std::string& newsMessage, const std::string& originExch) {
	printf( "News Bulletins. %d - Type: %d, Message: %s, Exchange of Origin: %s\n", msgId, msgType, newsMessage.c_str(), originExch.c_str());
}

void MClient::managedAccounts( const std::string& accountsList) {
	printf( "Account List: %s\n", accountsList.c_str());
}

void MClient::receiveFA(faDataType pFaDataType, const std::string& cxml) {
	std::cout << "Receiving FA: " << (int)pFaDataType << std::endl << cxml << std::endl;
}

void MClient::scannerParameters(const std::string& xml) {
	printf( "ScannerParameters. %s\n", xml.c_str());
}

void MClient::scannerData(int reqId, int rank, const ContractDetails& contractDetails,
                                const std::string& distance, const std::string& benchmark, const std::string& projection,
                                const std::string& legsStr) {
	printf( "ScannerData. %d - Rank: %d, Symbol: %s, SecType: %s, Currency: %s, Distance: %s, Benchmark: %s, Projection: %s, Legs String: %s\n", reqId, rank, contractDetails.contract.symbol.c_str(), contractDetails.contract.secType.c_str(), contractDetails.contract.currency.c_str(), distance.c_str(), benchmark.c_str(), projection.c_str(), legsStr.c_str());
}

void MClient::scannerDataEnd(int reqId) {
	printf( "ScannerDataEnd. %d\n", reqId);
}

void MClient::fundamentalData(TickerId reqId, const std::string& data) {
	printf( "FundamentalData. ReqId: %ld, %s\n", reqId, data.c_str());
}

void MClient::deltaNeutralValidation(int reqId, const DeltaNeutralContract& deltaNeutralContract) {
    printf( "DeltaNeutralValidation. %d, ConId: %ld, Delta: %s, Price: %s\n", reqId, deltaNeutralContract.conId, Utils::doubleMaxString(deltaNeutralContract.delta).c_str(), Utils::doubleMaxString(deltaNeutralContract.price).c_str());
}

void MClient::tickSnapshotEnd(int reqId) {
	printf( "TickSnapshotEnd: %d\n", reqId);
}

void MClient::marketDataType(TickerId reqId, int marketDataType) {
	printf( "MarketDataType. ReqId: %ld, Type: %d\n", reqId, marketDataType);
}

void MClient::commissionReport( const CommissionReport& commissionReport) {
    printf( "CommissionReport. %s - %s %s RPNL %s\n", commissionReport.execId.c_str(), Utils::doubleMaxString(commissionReport.commission).c_str(), commissionReport.currency.c_str(), Utils::doubleMaxString(commissionReport.realizedPNL).c_str());
}

void MClient::position( const std::string& account, const Contract& contract, Decimal position, double avgCost) {
    printf( "Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n", account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), decimalStringToDisplay(position).c_str(), Utils::doubleMaxString(avgCost).c_str());
}

void MClient::positionEnd() {
	printf( "PositionEnd\n");
}

void MClient::accountSummary( int reqId, const std::string& account, const std::string& tag, const std::string& value, const std::string& currency) {
	printf( "Acct Summary. ReqId: %d, Account: %s, Tag: %s, Value: %s, Currency: %s\n", reqId, account.c_str(), tag.c_str(), value.c_str(), currency.c_str());
}

void MClient::accountSummaryEnd( int reqId) {
	printf( "AccountSummaryEnd. Req Id: %d\n", reqId);
}

void MClient::verifyMessageAPI( const std::string& apiData) {
	printf("verifyMessageAPI: %s\b", apiData.c_str());
}

void MClient::verifyCompleted( bool isSuccessful, const std::string& errorText) {
	printf("verifyCompleted. IsSuccessfule: %d - Error: %s\n", isSuccessful, errorText.c_str());
}

void MClient::verifyAndAuthMessageAPI( const std::string& apiDatai, const std::string& xyzChallenge) {
	printf("verifyAndAuthMessageAPI: %s %s\n", apiDatai.c_str(), xyzChallenge.c_str());
}

void MClient::verifyAndAuthCompleted( bool isSuccessful, const std::string& errorText) {
	printf("verifyAndAuthCompleted. IsSuccessful: %d - Error: %s\n", isSuccessful, errorText.c_str());
    if (isSuccessful)
        m_Client->startApi();
}

void MClient::displayGroupList( int reqId, const std::string& groups) {
	printf("Display Group List. ReqId: %d, Groups: %s\n", reqId, groups.c_str());
}

void MClient::displayGroupUpdated( int reqId, const std::string& contractInfo) {
	std::cout << "Display Group Updated. ReqId: " << reqId << ", Contract Info: " << contractInfo << std::endl;
}

void MClient::positionMulti( int reqId, const std::string& account,const std::string& modelCode, const Contract& contract, Decimal pos, double avgCost) {
    printf("Position Multi. Request: %d, Account: %s, ModelCode: %s, Symbol: %s, SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n", reqId, account.c_str(), modelCode.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), decimalStringToDisplay(pos).c_str(), Utils::doubleMaxString(avgCost).c_str());
}

void MClient::positionMultiEnd( int reqId) {
	printf("Position Multi End. Request: %d\n", reqId);
}

void MClient::accountUpdateMulti( int reqId, const std::string& account, const std::string& modelCode, const std::string& key, const std::string& value, const std::string& currency) {
	printf("AccountUpdate Multi. Request: %d, Account: %s, ModelCode: %s, Key, %s, Value: %s, Currency: %s\n", reqId, account.c_str(), modelCode.c_str(), key.c_str(), value.c_str(), currency.c_str());
}

void MClient::accountUpdateMultiEnd( int reqId) {
	printf("Account Update Multi End. Request: %d\n", reqId);
}

void MClient::securityDefinitionOptionalParameter(int reqId, const std::string& exchange, int underlyingConId, const std::string& tradingClass,
                                                        const std::string& multiplier, const std::set<std::string>& expirations, const std::set<double>& strikes) {
	printf("Security Definition Optional Parameter. Request: %d, Trading Class: %s, Multiplier: %s\n", reqId, tradingClass.c_str(), multiplier.c_str());
}

void MClient::securityDefinitionOptionalParameterEnd(int reqId) {
	printf("Security Definition Optional Parameter End. Request: %d\n", reqId);
}

void MClient::softDollarTiers(int reqId, const std::vector<SoftDollarTier> &tiers) {
	printf("Soft dollar tiers (%lu):", tiers.size());

	for (unsigned int i = 0; i < tiers.size(); i++) {
		printf("%s\n", tiers[i].displayName().c_str());
	}
}

void MClient::familyCodes(const std::vector<FamilyCode> &familyCodes) {
	printf("Family codes (%lu):\n", familyCodes.size());

	for (unsigned int i = 0; i < familyCodes.size(); i++) {
		printf("Family code [%d] - accountID: %s familyCodeStr: %s\n", i, familyCodes[i].accountID.c_str(), familyCodes[i].familyCodeStr.c_str());
	}
}

void MClient::symbolSamples(int reqId, const std::vector<ContractDescription> &contractDescriptions) {
	printf("Symbol Samples (total=%lu) reqId: %d\n", contractDescriptions.size(), reqId);

	for (unsigned int i = 0; i < contractDescriptions.size(); i++) {
		Contract contract = contractDescriptions[i].contract;
		std::vector<std::string> derivativeSecTypes = contractDescriptions[i].derivativeSecTypes;
		printf("Contract (%u): conId: %ld, symbol: %s, secType: %s, primaryExchange: %s, currency: %s, ", i, contract.conId, contract.symbol.c_str(), contract.secType.c_str(), contract.primaryExchange.c_str(), contract.currency.c_str());
		printf("Derivative Sec-types (%lu):", derivativeSecTypes.size());
		for (unsigned int j = 0; j < derivativeSecTypes.size(); j++) {
			printf(" %s", derivativeSecTypes[j].c_str());
		}
		printf(", description: %s, issuerId: %s", contract.description.c_str(), contract.issuerId.c_str());
		printf("\n");
	}
}

void MClient::mktDepthExchanges(const std::vector<DepthMktDataDescription> &depthMktDataDescriptions) {
	printf("Mkt Depth Exchanges (%lu):\n", depthMktDataDescriptions.size());

	for (unsigned int i = 0; i < depthMktDataDescriptions.size(); i++) {
        printf("Depth Mkt Data Description [%d] - exchange: %s secType: %s listingExch: %s serviceDataType: %s aggGroup: %s\n", i,
            depthMktDataDescriptions[i].exchange.c_str(),
            depthMktDataDescriptions[i].secType.c_str(),
            depthMktDataDescriptions[i].listingExch.c_str(),
            depthMktDataDescriptions[i].serviceDataType.c_str(),
            Utils::intMaxString(depthMktDataDescriptions[i].aggGroup).c_str());
	}
}

void MClient::tickNews(int tickerId, time_t timeStamp, const std::string& providerCode, const std::string& articleId, const std::string& headline, const std::string& extraData) {
	printf("News Tick. TickerId: %d, TimeStamp: %s, ProviderCode: %s, ArticleId: %s, Headline: %s, ExtraData: %s\n", tickerId, ctime(&(timeStamp /= 1000)), providerCode.c_str(), articleId.c_str(), headline.c_str(), extraData.c_str());
}

void MClient::smartComponents(int reqId, const SmartComponentsMap& theMap) {
	printf("Smart components: (%lu):\n", theMap.size());

	for (SmartComponentsMap::const_iterator i = theMap.begin(); i != theMap.end(); i++) {
		printf(" bit number: %d exchange: %s exchange letter: %c\n", i->first, std::get<0>(i->second).c_str(), std::get<1>(i->second));
	}
}

void MClient::tickReqParams(int tickerId, double minTick, const std::string& bboExchange, int snapshotPermissions) {
    printf("tickerId: %d, minTick: %s, bboExchange: %s, snapshotPermissions: %u\n", tickerId, Utils::doubleMaxString(minTick).c_str(), bboExchange.c_str(), snapshotPermissions);

	// m_bboExchange = bboExchange;
}

void MClient::newsProviders(const std::vector<NewsProvider> &newsProviders) {
	printf("News providers (%lu):\n", newsProviders.size());

	for (unsigned int i = 0; i < newsProviders.size(); i++) {
		printf("News provider [%d] - providerCode: %s providerName: %s\n", i, newsProviders[i].providerCode.c_str(), newsProviders[i].providerName.c_str());
	}
}

void MClient::newsArticle(int requestId, int articleType, const std::string& articleText) {
	printf("News Article. Request Id: %d, Article Type: %d\n", requestId, articleType);
	if (articleType == 0) {
		printf("News Article Text (text or html): %s\n", articleText.c_str());
	} else if (articleType == 1) {
		std::string path;
		#if defined(IB_WIN32)
			TCHAR s[200];
			GetCurrentDirectory(200, s);
			path = s + std::string("\\MST$06f53098.pdf");
		#elif defined(IB_POSIX)
			char s[1024];
			if (getcwd(s, sizeof(s)) == NULL) {
				printf("getcwd() error\n");
				return;
			}
			path = s + std::string("/MST$06f53098.pdf");
		#endif
		std::vector<std::uint8_t> bytes = Utils::base64_decode(articleText);
		std::ofstream outfile(path, std::ios::out | std::ios::binary); 
		outfile.write((const char*)bytes.data(), bytes.size());
		printf("Binary/pdf article was saved to: %s\n", path.c_str());
	}
}

void MClient::historicalNews(int requestId, const std::string& time, const std::string& providerCode, const std::string& articleId, const std::string& headline) {
	printf("Historical News. RequestId: %d, Time: %s, ProviderCode: %s, ArticleId: %s, Headline: %s\n", requestId, time.c_str(), providerCode.c_str(), articleId.c_str(), headline.c_str());
}

void MClient::historicalNewsEnd(int requestId, bool hasMore) {
	printf("Historical News End. RequestId: %d, HasMore: %s\n", requestId, (hasMore ? "true" : " false"));
}

void MClient::headTimestamp(int reqId, const std::string& headTimestamp) {
	printf( "Head time stamp. ReqId: %d - Head time stamp: %s,\n", reqId, headTimestamp.c_str());

}

void MClient::histogramData(int reqId, const HistogramDataVector& data) {
	printf("Histogram. ReqId: %d, data length: %lu\n", reqId, data.size());

    for (const HistogramEntry& entry : data) {
        printf("\t price: %s, size: %s\n", Utils::doubleMaxString(entry.price).c_str(), decimalStringToDisplay(entry.size).c_str());
	}
}

void MClient::rerouteMktDataReq(int reqId, int conid, const std::string& exchange) {
	printf( "Re-route market data request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
}

void MClient::rerouteMktDepthReq(int reqId, int conid, const std::string& exchange) {
	printf( "Re-route market depth request. ReqId: %d, ConId: %d, Exchange: %s\n", reqId, conid, exchange.c_str());
}

void MClient::marketRule(int marketRuleId, const std::vector<PriceIncrement> &priceIncrements) {
    printf("Market Rule Id: %s\n", Utils::intMaxString(marketRuleId).c_str());
    for (unsigned int i = 0; i < priceIncrements.size(); i++) {
        printf("Low Edge: %s, Increment: %s\n", Utils::doubleMaxString(priceIncrements[i].lowEdge).c_str(), Utils::doubleMaxString(priceIncrements[i].increment).c_str());
    }
}

void MClient::pnl(int reqId, double dailyPnL, double unrealizedPnL, double realizedPnL) {
    printf("PnL. ReqId: %d, daily PnL: %s, unrealized PnL: %s, realized PnL: %s\n", reqId, Utils::doubleMaxString(dailyPnL).c_str(), Utils::doubleMaxString(unrealizedPnL).c_str(), 
        Utils::doubleMaxString(realizedPnL).c_str());
}

void MClient::pnlSingle(int reqId, Decimal pos, double dailyPnL, double unrealizedPnL, double realizedPnL, double value) {
    printf("PnL Single. ReqId: %d, pos: %s, daily PnL: %s, unrealized PnL: %s, realized PnL: %s, value: %s\n", reqId, decimalStringToDisplay(pos).c_str(), Utils::doubleMaxString(dailyPnL).c_str(), 
        Utils::doubleMaxString(unrealizedPnL).c_str(), Utils::doubleMaxString(realizedPnL).c_str(), Utils::doubleMaxString(value).c_str());
}

void MClient::historicalTicks(int reqId, const std::vector<HistoricalTick>& ticks, bool done) {
    for (const HistoricalTick& tick : ticks) {
    std::time_t t = tick.time;
        std::cout << "Historical tick. ReqId: " << reqId << ", time: " << ctime(&t) << ", price: "<< Utils::doubleMaxString(tick.price).c_str()	<< ", size: " << decimalStringToDisplay(tick.size).c_str() << std::endl;
    }
}

void MClient::historicalTicksBidAsk(int reqId, const std::vector<HistoricalTickBidAsk>& ticks, bool done) {
    for (const HistoricalTickBidAsk& tick : ticks) {
    std::time_t t = tick.time;
        std::cout << "Historical tick bid/ask. ReqId: " << reqId << ", time: " << ctime(&t) << ", price bid: "<< Utils::doubleMaxString(tick.priceBid).c_str()	<<
            ", price ask: "<< Utils::doubleMaxString(tick.priceAsk).c_str() << ", size bid: " << decimalStringToDisplay(tick.sizeBid).c_str() << ", size ask: " << decimalStringToDisplay(tick.sizeAsk).c_str() <<
            ", bidPastLow: " << tick.tickAttribBidAsk.bidPastLow << ", askPastHigh: " << tick.tickAttribBidAsk.askPastHigh << std::endl;
    }
}

void MClient::historicalTicksLast(int reqId, const std::vector<HistoricalTickLast>& ticks, bool done) {
    for (HistoricalTickLast tick : ticks) {
	std::time_t t = tick.time;
        std::cout << "Historical tick last. ReqId: " << reqId << ", time: " << ctime(&t) << ", price: "<< Utils::doubleMaxString(tick.price).c_str() <<
            ", size: " << decimalStringToDisplay(tick.size).c_str() << ", exchange: " << tick.exchange << ", special conditions: " << tick.specialConditions <<
            ", unreported: " << tick.tickAttribLast.unreported << ", pastLimit: " << tick.tickAttribLast.pastLimit << std::endl;
    }
}

void MClient::tickByTickAllLast(int reqId, int tickType, time_t time, double price, Decimal size, const TickAttribLast& tickAttribLast, const std::string& exchange, const std::string& specialConditions) {
    printf("Tick-By-Tick. ReqId: %d, TickType: %s, Time: %s, Price: %s, Size: %s, PastLimit: %d, Unreported: %d, Exchange: %s, SpecialConditions:%s\n", 
        reqId, (tickType == 1 ? "Last" : "AllLast"), ctime(&time), Utils::doubleMaxString(price).c_str(), decimalStringToDisplay(size).c_str(), tickAttribLast.pastLimit, tickAttribLast.unreported, exchange.c_str(), specialConditions.c_str());
}

void MClient::tickByTickBidAsk(int reqId, time_t time, double bidPrice, double askPrice, Decimal bidSize, Decimal askSize, const TickAttribBidAsk& tickAttribBidAsk) {
    printf("Tick-By-Tick. ReqId: %d, TickType: BidAsk, Time: %s, BidPrice: %s, AskPrice: %s, BidSize: %s, AskSize: %s, BidPastLow: %d, AskPastHigh: %d\n", 
        reqId, ctime(&time), Utils::doubleMaxString(bidPrice).c_str(), Utils::doubleMaxString(askPrice).c_str(), decimalStringToDisplay(bidSize).c_str(), decimalStringToDisplay(askSize).c_str(), tickAttribBidAsk.bidPastLow, tickAttribBidAsk.askPastHigh);
}

void MClient::tickByTickMidPoint(int reqId, time_t time, double midPoint) {
    printf("Tick-By-Tick. ReqId: %d, TickType: MidPoint, Time: %s, MidPoint: %s\n", reqId, ctime(&time), Utils::doubleMaxString(midPoint).c_str());
}

void MClient::orderBound(long long orderId, int apiClientId, int apiOrderId) {
    printf("Order bound. OrderId: %s, ApiClientId: %s, ApiOrderId: %s\n", Utils::llongMaxString(orderId).c_str(), Utils::intMaxString(apiClientId).c_str(), Utils::intMaxString(apiOrderId).c_str());
}

void MClient::replaceFAEnd(int reqId, const std::string& text) {
	printf("Replace FA End. Request: %d, Text:%s\n", reqId, text.c_str());
}

void MClient::wshMetaData(int reqId, const std::string& dataJson) {
	printf("WSH Meta Data. ReqId: %d, dataJson: %s\n", reqId, dataJson.c_str());
}

void MClient::wshEventData(int reqId, const std::string& dataJson) {
	printf("WSH Event Data. ReqId: %d, dataJson: %s\n", reqId, dataJson.c_str());
}

void MClient::historicalSchedule(int reqId, const std::string& startDateTime, const std::string& endDateTime, const std::string& timeZone, const std::vector<HistoricalSession>& sessions) {
	printf("Historical Schedule. ReqId: %d, Start: %s, End: %s, TimeZone: %s\n", reqId, startDateTime.c_str(), endDateTime.c_str(), timeZone.c_str());
	for (unsigned int i = 0; i < sessions.size(); i++) {
		printf("\tSession. Start: %s, End: %s, RefDate: %s\n", sessions[i].startDateTime.c_str(), sessions[i].endDateTime.c_str(), sessions[i].refDate.c_str());
	}
}

void MClient::userInfo(int reqId, const std::string& whiteBrandingId) {
    printf("User Info. ReqId: %d, WhiteBrandingId: %s\n", reqId, whiteBrandingId.c_str());
}


