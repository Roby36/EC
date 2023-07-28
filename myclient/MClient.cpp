
#include "MClient.h"

MClient::MClient( const std::string logPath, ReqIds reqIds, bool init_trade_data) :
	  m_state(ST_CONNECT)		
	, m_logger(new Mlogger( logPath))
	, m_orderId(0)
	, m_extraAuth(false)
	, m_osSignal(2000)	// 2-seconds timeout
	, m_Client(new EClientSocket(this, &m_osSignal)) // Finally pass instance to EClientSocket
	, m_reqIds(reqIds)
{
	m_tradeData = (TradeData *) malloc (sizeof(TradeData));
	for (int i = 0; i < MAXTRADES; i++) {
		m_tradeData->tradeArr[i] = (MTrade_t*) malloc (sizeof(MTrade_t));
	}
	rec_order = (Order *) malloc (sizeof(Order));
	if (!init_trade_data)
		archive_td_in(); //! Restore trade Data: cannot be called with empty ser file!!!
}

MClient::~MClient()
{
	for (int i = 0; i < m_instr_Id; i++) {
		if (m_instrArray[i] != NULL) 
			delete(m_instrArray[i]);	
	}
	for (int i = 0; i < MAXTRADES; i++) {
		free(m_tradeData->tradeArr[i]);
	}	
	free(m_tradeData);
	free(rec_order);
	delete m_Reader;
	delete m_Client;
	delete m_logger;
}

//*** SERIALIZATION ***//
void MClient::archive_td_out()
{
	std::ofstream out(serFileName);
	boost::archive::text_oarchive text_output_archive(out);
	text_output_archive & (*this->m_tradeData);
	out.close();
}

void MClient::archive_td_in()
{
	std::ifstream in (serFileName);
	boost::archive::text_iarchive text_input_archive(in);
	text_input_archive & (*this->m_tradeData);
	in.close();
}

void MClient::testSerFile()
{
	m_logger->str(std::string("Number of trades: ") + std::to_string(m_tradeData->numTrades) + std::string("\n"));
	for (int i = 0; i < m_tradeData->numTrades; i++) {
		MTrade_t* currTrade = m_tradeData->tradeArr[i];
		// Print trade details
		m_logger->str(
			std::string("\tTrade number ")          + std::to_string(i) 							  + std::string("\n") +
		    std::string("Trade Id: ") 			    + std::to_string(currTrade->tradeId) 			  + std::string("\n") +
		    std::string("Strategy: ") 			    + currTrade->strategy 							  + std::string("\n") +
		    std::string("Instrument: ") 			+ std::to_string(currTrade->instr_id) 		      + std::string("\n") +
		    std::string("Opening order Id: ")       + std::to_string(currTrade->openingOrder.orderId) + std::string("\n") +
		    std::string("Opening order action: ")   + currTrade->openingOrder.action 		          + std::string("\n") +
			std::string("Closing order Id: ")       + std::to_string(currTrade->closingOrder.orderId) + std::string("\n") +
		    std::string("Closing order action: ")   + currTrade->closingOrder.action		          + std::string("\n") + 
			std::string("Opening execution Id:")    + currTrade->openingExecution.execId  			  + std::string("\n") + 
			std::string("Opening execution time: ") + currTrade->openingExecution.time			      + std::string("\n") +
			std::string("Closing execution Id:")    + currTrade->closingExecution.execId			  + std::string("\n") + 
			std::string("Closing execution time: ") + currTrade->closingExecution.time			      + std::string("\n") +
			std::string("\tTrade number: ")         + std::to_string(i) 						      + std::string(" end\n")
		);
	}	
}

/*** PRIVATE ECLIENT REQUEST METHODS ***/

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

bool MClient::reqIds(int numIds) {
	m_state = REQIDS;
	m_Client->reqIds(numIds);
	return waitResponse(REQIDS_ACK, numIds, "reqIds()");
}

void MClient::handle_nextValidId( OrderId orderId) {
	m_state = REQIDS_ACK;
	m_logger->str( "Next Valid Id: " + std::to_string(orderId) + "\n");
	m_orderId = orderId;
}

bool MClient::reqContractDetails(int reqId, Contract contract) {
	m_state = REQCONTRACTDETAILS;
	m_Client->reqContractDetails( reqId, contract);
	return waitResponse(REQCONTRACTDETAILS_ACK, reqId, "reqContractDetails()");
}

void MClient::handle_contractDetailsEnd(int reqId) {
#ifndef MEMDBG
	m_logger->str( "ContractDetailsEnd: reqId: " + std::to_string(reqId) + "\n");
#endif
	this->m_state = REQCONTRACTDETAILS_ACK;
}

bool MClient::reqHistoricalData( int tickerId, Contract contract, std::string endDateTime,
								std::string durationStr, std::string barSizeSetting, std::string whatToShow,
						        int useRTH, int formatDate, bool keepUpToDate,
								std::shared_ptr<TagValueList> chartOptions) {
	m_state = REQHISTORICALDATA;
	m_Client->reqHistoricalData( tickerId, contract, endDateTime, durationStr, barSizeSetting, whatToShow, useRTH, formatDate, keepUpToDate, chartOptions);
	return waitResponse(REQHISTORICALDATA_ACK, tickerId, "reqHistoricalData()");
}

void MClient::handle_historicalDataEnd(int reqId, const std::string& startDateStr, const std::string& endDateStr) {
	this->m_state = REQHISTORICALDATA_ACK;
	m_logger->str( "HistoricalDataEnd. ReqId: " + std::to_string(reqId) + " - Start Date: " + startDateStr + ", End Date: " + endDateStr + "\n" );
}

bool MClient::reqAllOpenOrders() {
	m_state = REQALLOPENORDERS;
	m_Client->reqAllOpenOrders();
	return waitResponse(REQALLOPENORDERS_ACK, -1, "reqAllOpenOrders()");
}

void MClient::handle_openOrderEnd() {
	m_logger->str( "OpenOrderEnd\n");
	m_state = REQALLOPENORDERS_ACK;
}

bool MClient::reqCompletedOrders( bool apiOnly) {
	m_state = REQCOMPLETEDORDERS;
	m_Client->reqCompletedOrders( apiOnly);
	return waitResponse(REQCOMPLETEDORDERS_ACK, -1, "reqCompletedOrders()");
}

void MClient::handle_completedOrdersEnd() {
	m_logger->str( "CompletedOrdersEnd\n");
	m_state = REQCOMPLETEDORDERS_ACK;
}

void MClient::reqAutoOpenOrders( bool autoBind) {
	m_Client->reqAutoOpenOrders( autoBind);
}

bool MClient::reqRealTimeBars( int 	    tickerId,
							   Contract contract,
							   int 	    barSize,
							   std::string 	whatToShow,
							   bool 	useRTH,
						       std::shared_ptr<TagValueList> realTimeBarsOptions) {
	m_state = REQREALTIMEBARS;
	m_Client->reqRealTimeBars( tickerId, contract, barSize, whatToShow, useRTH, realTimeBarsOptions);
	return waitResponse( REQREALTIMEBARS_ACK, tickerId, "reqRealTimeBars()");
}

bool MClient::reqPositions() {
	m_state = REQPOSITIONS;
	m_Client->reqPositions();
	return waitResponse( REQPOSITIONS_ACK, -1, "reqPositions()");
}

void MClient::handle_positionEnd() {
	m_logger->str( "PositionEnd\n");
	m_state = REQPOSITIONS_ACK;
}

bool MClient::reqExecutions( ExecutionFilter execFilter) {
	m_state = REQEXECUTIONS;
	m_Client->reqExecutions( this->m_reqIds.reqExecutions, execFilter);
	return waitResponse( REQEXECUTIONS_ACK, this->m_reqIds.reqExecutions, "reqExecutions()");
}

void MClient::handle_execDetailsEnd( int reqId) {
	m_logger->str( "ExecDetailsEnd. reqId: " + std::to_string( reqId) +  std::string("\n") );
	m_state = REQEXECUTIONS_ACK;
}

/*** STILL TO IMPLEMENT ****/
void MClient::cancelRealTimeBars(int tickerId) {
	m_Client->cancelRealTimeBars(tickerId);
}

//** RECEIVAL CONFIRMATION METHOD **//
bool MClient::waitResponse( State target, int reqId, std::string caller) {
	time_t start_time = time(NULL);
	// Iterate until time runs out
	while (	time(NULL) - start_time < req_timeout) {
		// If goal state reached, return
		if (m_state == target) {
			m_logger->str("\t Id " + std::to_string(reqId) + " from " + caller 
							+ " returned successfully\n");
			return true;
		}
		// Keep quering other thread
		m_osSignal.waitForSignal();
		errno = 0;
		m_Reader->processMsgs();
	}
	m_logger->str("\t Id " + std::to_string(reqId) + " from " + caller 
					+ " timed out\n");
	return false;
}

/*** TRADES & ORDERS ***/

int MClient::placeOrder( int inst_id, Order order) {
	// Attemp to retrieve the given instrument
	Instrument* instr = get_Instrument(inst_id);
	if (instr == NULL) {
		m_logger->str("\t\n Cannot place order: invalid inst_id\n\n");
		return -1;
	}
	// Request and update next valid orderId
	if (!reqIds()) {
		m_logger->str("\t\n Cannot place order: error receiving next valid orderId\n\n");
		return -1;
	}
	m_state = PLACEORDER;
	m_Client->placeOrder( m_orderId, instr->orderContract.contract, order);
	if ( !waitResponse( PLACEORDER_ACK, m_orderId, "placeOrder()"))
		return -1;
	return m_orderId;
}

void MClient::handle_openOrder( OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
	m_logger->openOrder( orderId, contract, order, orderState);
	// Mark successful order execution only if orderId corresponds with the current one & order recently placed
	if ( m_state == PLACEORDER && m_orderId == orderId) {
		memcpy(rec_order, &order, sizeof(Order));
		m_state = PLACEORDER_ACK;
	}
	// Handle open orders information receival
	if (m_state == REQALLOPENORDERS)
		m_logger->str( "\tReceived orderId " + std::to_string(order.orderId) + " orderRef " + order.orderRef +"\n");
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

void MClient::handle_orderStatus(OrderId orderId, const std::string& status, Decimal filled,
		Decimal remaining, double avgFillPrice, int permId, int parentId,
		double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) {
	
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

void MClient::setTradingState(TradingState tstate)
{
	// Set EACH strategy for EACH instrument to the given state
	for (int i = 0; i < m_instr_Id; i++) {
		for (int s = 0; s < m_stratCount[i]; s++) {
			Strategy * curr_strat = m_stratArray[i][s];
			curr_strat->t_state = tstate;
		}
	}
}

int MClient::openTrade(Strategy * strategy)
{
	MTrade_t * currTrade = strategy->trade2open;
	// Place opening order (fetch execution details from callback later on)
	if (placeOrder(currTrade->instr_id, currTrade->openingOrder) == -1) {
		m_logger->str("openTrade error: failed openingOrder.\n");
		return -1;
	}
	m_logger->str(std::string("Successfully placed order to open trade ") 	            + 
				  std::to_string(currTrade->tradeId)  + std::string(" for instrument ") +
				  std::to_string(currTrade->instr_id) + std::string(" for strategy ")   + strategy->strategy_code +
				  std::string(" OrderRef: " + currTrade->openingOrder.orderRef + std::string("\n")));
	memcpy(&currTrade->openingOrder, this->rec_order, sizeof(Order)); // save full opening order retrieved from callback
	memcpy(m_tradeData->tradeArr[m_tradeData->numTrades++], currTrade, sizeof(MTrade_t) );
	archive_td_out(); // update trade archive
	strategy->openingTrade = false; // signal strategy
	return (m_tradeData->numTrades - 1); // return previous trade Id
}

bool MClient::closeTrade(Strategy * strategy)
{
	// Save the trade to close
	MTrade_t * currTrade = strategy->trade2close;
	// Place closing order (& fetch execution details from callback!)
	if (placeOrder(currTrade->instr_id, currTrade->closingOrder) == -1) {
		m_logger->str("closeTrade error: failed closingOrder.\n");
		return false;
	}
	m_logger->str(std::string("Successfully placed order to close trade ")        + 
				  std::to_string(currTrade->tradeId)  + std::string(" for instrument ") +
				  std::to_string(currTrade->instr_id) + std::string(" for strategy ")   + strategy->strategy_code +
				  std::string(" OrderRef: " + currTrade->closingOrder.orderRef + std::string("\n")));
	memcpy(&currTrade->closingOrder, this->rec_order, sizeof(Order)); // save full closing order retrieved from callback
	memcpy(m_tradeData->tradeArr[currTrade->tradeId], currTrade, sizeof(MTrade_t)); // Update trade entry
	archive_td_out(); // update trade archive 
	strategy->closingTrade = false; // signal strategy
	return true;
}

void MClient::handle_execDetails(int reqId, const Contract& contract, const Execution& execution) {
	m_logger->execDetails(reqId, contract, execution);
	// Iterate through each trade
	for (int i = 0; i < m_tradeData->numTrades; i++) {
		MTrade_t* currTrade = m_tradeData->tradeArr[i];
		// Check unexecuted trades to open
		if (!currTrade->isOpen && currTrade->openingOrder.permId == execution.permId) {
			currTrade->openingExecution = execution;
			currTrade->isOpen = true;
			// Log execution
			m_logger->str(std::string("Successfully executed order to open trade ") 	        + 
						  std::to_string(currTrade->tradeId)  + std::string(" for instrument ") +
						  std::to_string(currTrade->instr_id) + std::string(" for strategy ")   + currTrade->strategy +
						  std::string(" OrderRef: " + currTrade->openingOrder.orderRef + std::string("\n")));
		// Check unexecuted trades to close
		} else if (currTrade->isOpen && currTrade->closingOrder.permId == execution.permId) {
			currTrade->closingExecution = execution;
			currTrade->isOpen = false;
			// Log execution
			m_logger->str(std::string("Successfully executed order to close trade ") 	        + 
						  std::to_string(currTrade->tradeId)  + std::string(" for instrument ") +
						  std::to_string(currTrade->instr_id) + std::string(" for strategy ")   + currTrade->strategy +
						  std::string(" OrderRef: " + currTrade->openingOrder.orderRef + std::string("\n")));
		}
	}
}

void MClient::handle_historicalData(TickerId reqId, const Bar& bar) 
{	
#ifndef MEMDBG
    m_logger->historicalData(reqId, bar);
#endif
	// Identify Instrument corresponding to given reqId
	Instrument * instr = reqId_Instrument((int)reqId);
	if (instr == NULL) {
		m_logger->str("\n\t ERROR: historicalDataUpdate reqId not found\n\n");
		return;
	}
	// Try to update the instrument's Bars, and check if instrument requires bar update
	if (!instr->addBar(reqId, bar)) {
		if (instr->bars2update != 0) {
			update_instr_bars(instr->inst_id, instr->bars2update, false);
			instr->bars2update = 0; // set flag back to zero
		}
		return;
	}
	// Call each strategy on the instrument
	for (int s = 0; s < m_stratCount[instr->inst_id]; s++) {
		Strategy * curr_strat = m_stratArray[instr->inst_id][s];
		curr_strat->handle_barUpdate();
		// Check if a trade needs to be opened or closed
		if (curr_strat->openingTrade)
			openTrade(curr_strat);
		else if (curr_strat->closingTrade)
			closeTrade(curr_strat);
	}
}

void MClient::handle_realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                            Decimal volume, Decimal wap, int count) {
	
    m_logger->realtimeBar( reqId, time, open, high, low, close, volume, wap, count);
	// Acknowledge receival of FIRST BAR only
	if (m_state == REQREALTIMEBARS)
		m_state =  REQREALTIMEBARS_ACK;
	// Identify Instrument corresponding to given reqId
	Instrument* instr = reqId_Instrument((int)reqId);
	if ( instr == NULL) {
		m_logger->str("\n\t ERROR: realtimeBar reqId not found\n\n");
		return;
	}
	int instr_id = instr->inst_id;
	// Call each strategy on the instrument
	for (int s = 0; s < m_stratCount[instr_id]; s++) {
		Strategy * curr_strat = m_stratArray[instr_id][s];
		curr_strat->handle_realTimeBar(close);
		// Check if a trade needs to be closed (only option for real time bars)
		if (curr_strat->closingTrade)
			closeTrade(curr_strat);
	}
}

//** BACKTESTS **//
void MClient::print_backtests()
{
	for (int i = 0; i < m_instr_Id; i++) {
		Instrument* instr = m_instrArray[i];
		int instr_id = instr->inst_id;
		for (int s = 0; s < m_stratCount[instr_id]; s++) {
			Strategy * curr_strat = m_stratArray[instr_id][s];
			m_logger->str("Printing backtests for instr " + std::to_string(instr_id) + " strategy " + curr_strat->strategy_code + "\n");
			curr_strat->print_backtest();
		}
    }
}

/*** HANDLING INSTRUMENTS ***/

int MClient::add_Instrument(const std::string barSize, 
							ContractDetails dataContract, 
							ContractDetails orderContract, 
							Instrument::ReqIds reqIds, 
							std::string logPath) 
{
	if ( m_instr_Id < MAXINSTR) {
		// First initialize new instrument & add to array
		m_instrArray[m_instr_Id] = new Instrument( m_instr_Id, 
												   barSize, 
												   dataContract,
												   orderContract, 
												   reqIds, 
												   logPath
												 );
		return m_instr_Id++; // return & increment current instrument Id
	}
	// Handle failure
	m_logger->str("\t Cannot add instrument: maximum number of instruments reached.\n");
	return -1;
}

void MClient::add_Strategy(const int instr_id, Strategy * strategy)
{
	Instrument * instr = get_Instrument(instr_id);
	if (instr == NULL) {
		m_logger->str(" add_Strategy(): instrument out of range\n");
		return;
	}
	m_stratArray[instr_id][(m_stratCount[instr_id])++] = strategy;
	m_logger->str("Added Strategy " + m_stratArray[instr_id][(m_stratCount[instr_id]) - 1]->strategy_code);
}

Instrument* MClient::get_Instrument(int instr_Id) {
	if ( instr_Id >= 0 && instr_Id < m_instr_Id)
		return m_instrArray[instr_Id];
	m_logger->str("\n\t ERROR: Instrument " + std::to_string(instr_Id) + " out of range\n\n");
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

void MClient::handle_contractDetails(int reqId, const ContractDetails& contractDetails) {
#ifndef MEMDBG
	m_logger->contractDetails(reqId, contractDetails);
#endif
	Instrument* instr = reqId_Instrument(reqId); // identify Instrument corresponding to given reqId
	if ( instr == NULL) {
		m_logger->str("\n\t ERROR: conractDetails reqId not found\n\n");
		return;
	}
	instr->updateDataContract (reqId, contractDetails);
	instr->updateOrderContract(reqId, contractDetails);
}

Instrument* MClient::reqId_Instrument( int reqId) {
	for (int i = 0; i < m_instr_Id; i++) {
		if ( m_instrArray[i]->m_reqIds.dataContract   == reqId
		  || m_instrArray[i]->m_reqIds.orderContract  == reqId
		  || m_instrArray[i]->m_reqIds.realTimeBars   == reqId
		  || m_instrArray[i]->m_reqIds.historicalBars == (TickerId) reqId
		  || m_instrArray[i]->m_reqIds.updatedBars    == (TickerId) reqId)
			return m_instrArray[i];
	}
	return NULL;
}

//** BARS **//

void MClient::update_instr_bars(int instr_id, int numBars, bool initialization, std::string whatToShow, int useRTH) {
	Instrument* instr;
	if (instr_id >= m_instr_Id || (instr = m_instrArray[instr_id]) == NULL) {
		m_logger->str( "MClient::update_instr_bars(): invalid instrument id\n");
		return;
	}
	if (!initialization && !instr->requires_update()) // if we are in update mode check if instrument requires update
		return;
	char* durStr = instr->extend_dur(numBars);
	if (durStr == NULL) {
		m_logger->str( "\tMClient::update_instr_bars(): extend_dur() error for instrument " + std::to_string(instr_id) + "\n");
		return;
	}
	char* queryTime = currTime_str();
	// Fetch appropriate reqIds for instrument
	int reqId = (initialization == true)  * ((int) instr->m_reqIds.historicalBars)
			  + (initialization == false) * ((int) instr->m_reqIds.updatedBars);
	if ( !reqHistoricalData(reqId, instr->dataContract.contract, queryTime,
							durStr, instr->barSize, whatToShow, useRTH, 1, false))
		m_logger->str("\tError retrieving / updating historical data for instrument " + std::to_string(instr_id) + "\n");
	delete(queryTime); // clean up
	delete(durStr);
}

void MClient::update_bars( int numBars, bool initialization, std::string whatToShow, int useRTH) {
	for (int i = 0; i < m_instr_Id; i++) {
		update_instr_bars(i, numBars, initialization, whatToShow, useRTH);
	}
}

/*** MALLOC'D STRING MUST BE FREE'D ***/
char* MClient::currTime_str() {
	std::time_t rawtime;
    std::tm* timeinfo;
    char* queryTime = new char[80];
	std::time(&rawtime);
    timeinfo = std::gmtime(&rawtime);
	std::strftime(queryTime, 80, "%Y%m%d-%H:%M:%S", timeinfo);
	return queryTime;
}

/******** EWRAPPER CALLBACKS HANDLERS *******/

void MClient::handle_currentTime( long time) {
	m_logger->currentTime(time);
}

void MClient::handle_error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson) {
	m_logger->error(id, errorCode, errorString, advancedOrderRejectJson);
}

void MClient::handle_historicalDataUpdate(TickerId reqId, const Bar& bar) {
    m_logger->historicalData(reqId, bar);
}

void MClient::handle_completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {
    m_logger->completedOrder( contract, order, orderState);
}

void MClient::handle_position( const std::string& account, const Contract& contract, Decimal position, double avgCost) {
    m_logger->position( account, contract, position, avgCost);
}
