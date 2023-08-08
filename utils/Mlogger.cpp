
#include "Mlogger.h"

Mlogger::Mlogger(std::string filepath) {
	if (filepath == "") {
		lfp = stderr;
		return;
	}
    lfp = fopen(filepath.c_str(), "a");
    if (lfp == NULL) {
        fprintf(stderr, "Error opening log file \n");
    }
    time_t now = ::time(NULL);
    struct tm * timeinfo = localtime ( &now);
    fprintf( lfp, "\tLOG INITIATED AT: %s\n", asctime( timeinfo));
    fprintf( stdout, "Initiated log file %s\n", filepath.c_str());
	fflush(lfp);
}

Mlogger::~Mlogger() {
    fclose(this->lfp);
}

void Mlogger::str(std::string str) {
    fprintf(lfp, "%s", str.c_str());
	fflush(lfp);
}

void Mlogger::currentTime( long time) {
    time_t t = ( time_t)time;
    struct tm * timeinfo = localtime ( &t);
    fprintf( lfp, "The current date/time is: %s", asctime( timeinfo));
	fflush(lfp);
}

void Mlogger::error(int id, int errorCode, const std::string& errorString, const std::string& advancedOrderRejectJson) {
    if (!advancedOrderRejectJson.empty()) {
        fprintf(lfp, "Error. Id: %d, Code: %d, Msg: %s, AdvancedOrderRejectJson: %s\n", id, errorCode, errorString.c_str(), advancedOrderRejectJson.c_str());
    } else {
        fprintf(lfp, "Error. Id: %d, Code: %d, Msg: %s\n", id, errorCode, errorString.c_str());
    }
	fflush(lfp);
}

void Mlogger::orderStatus(OrderId orderId, const std::string& status, Decimal filled,
    Decimal remaining, double avgFillPrice, int permId, int parentId,
    double lastFillPrice, int clientId, const std::string& whyHeld, double mktCapPrice) {
        fprintf(lfp, "OrderStatus. Id: %ld, Status: %s, Filled: %s, Remaining: %s, AvgFillPrice: %s, PermId: %s, LastFillPrice: %s, ClientId: %s, WhyHeld: %s, MktCapPrice: %s\n", 
        orderId, status.c_str(), decimalStringToDisplay(filled).c_str(), decimalStringToDisplay(remaining).c_str(), Utils::doubleMaxString(avgFillPrice).c_str(), Utils::intMaxString(permId).c_str(), 
        Utils::doubleMaxString(lastFillPrice).c_str(), Utils::intMaxString(clientId).c_str(), whyHeld.c_str(), Utils::doubleMaxString(mktCapPrice).c_str());
	fflush(lfp);
}

void Mlogger::openOrder( OrderId orderId, const Contract& contract, const Order& order, const OrderState& orderState) {
    fprintf( lfp, "OpenOrder. PermId: %s, ClientId: %s, OrderId: %s, Account: %s, Symbol: %s, SecType: %s, Exchange: %s:, Action: %s, OrderType:%s, TotalQty: %s, CashQty: %s, "
        "LmtPrice: %s, AuxPrice: %s, Status: %s, MinTradeQty: %s, MinCompeteSize: %s, CompeteAgainstBestOffset: %s, MidOffsetAtWhole: %s, MidOffsetAtHalf: %s\n", 
        Utils::intMaxString(order.permId).c_str(), Utils::longMaxString(order.clientId).c_str(), Utils::longMaxString(orderId).c_str(), order.account.c_str(), contract.symbol.c_str(), 
        contract.secType.c_str(), contract.exchange.c_str(), order.action.c_str(), order.orderType.c_str(), decimalStringToDisplay(order.totalQuantity).c_str(), 
        Utils::doubleMaxString(order.cashQty).c_str(), Utils::doubleMaxString(order.lmtPrice).c_str(), Utils::doubleMaxString(order.auxPrice).c_str(), orderState.status.c_str(),
        Utils::intMaxString(order.minTradeQty).c_str(), Utils::intMaxString(order.minCompeteSize).c_str(), 
        order.competeAgainstBestOffset == COMPETE_AGAINST_BEST_OFFSET_UP_TO_MID ? "UpToMid" : Utils::doubleMaxString(order.competeAgainstBestOffset).c_str(),
        Utils::doubleMaxString(order.midOffsetAtWhole).c_str(), Utils::doubleMaxString(order.midOffsetAtHalf).c_str());
		fflush(lfp);
}

void Mlogger::contractDetails(int reqId, const ContractDetails& contractDetails) {
    fprintf( lfp, "ContractDetails begin. ReqId: %d\n", reqId);
	printContractMsg(contractDetails.contract);
	printContractDetailsMsg(contractDetails);
	fprintf( lfp, "ContractDetails end. ReqId: %d\n", reqId);
	fflush(lfp);
}

void Mlogger::historicalData(TickerId reqId, const Bar& bar) {
    fprintf( lfp, "HistoricalData. ReqId: %ld - Date: %s, Open: %s, High: %s, Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n", reqId, bar.time.c_str(), 
        Utils::doubleMaxString(bar.open).c_str(), Utils::doubleMaxString(bar.high).c_str(), Utils::doubleMaxString(bar.low).c_str(), Utils::doubleMaxString(bar.close).c_str(), 
        decimalStringToDisplay(bar.volume).c_str(), Utils::intMaxString(bar.count).c_str(), decimalStringToDisplay(bar.wap).c_str());
	fflush(lfp);
}

void Mlogger::realtimeBar(TickerId reqId, long time, double open, double high, double low, double close,
                        Decimal volume, Decimal wap, int count) {
                            fprintf( lfp, "RealTimeBars. %ld - Time: %s, Open: %s, High: %s, Low: %s, Close: %s, Volume: %s, Count: %s, WAP: %s\n", reqId, Utils::longMaxString(time).c_str(), 
        Utils::doubleMaxString(open).c_str(), Utils::doubleMaxString(high).c_str(), Utils::doubleMaxString(low).c_str(), Utils::doubleMaxString(close).c_str(), 
        decimalStringToDisplay(volume).c_str(), Utils::intMaxString(count).c_str(), decimalStringToDisplay(wap).c_str());
		fflush(lfp);
}

void Mlogger::completedOrder(const Contract& contract, const Order& order, const OrderState& orderState) {
    fprintf( lfp, "CompletedOrder: orderId: %s PermId: %s, ParentPermId: %s, Account: %s, Symbol: %s, SecType: %s, Exchange: %s:, Action: %s, OrderType: %s, TotalQty: %s, CashQty: %s, FilledQty: %s, "
        "LmtPrice: %s, AuxPrice: %s, Status: %s, CompletedTime: %s, CompletedStatus: %s, MinTradeQty: %s, MinCompeteSize: %s, CompeteAgainstBestOffset: %s, MidOffsetAtWhole: %s, MidOffsetAtHalf: %s\n",
		Utils::longMaxString(order.orderId).c_str(),
        Utils::intMaxString(order.permId).c_str(), Utils::llongMaxString(order.parentPermId).c_str(), order.account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), contract.exchange.c_str(),
        order.action.c_str(), order.orderType.c_str(), decimalStringToDisplay(order.totalQuantity).c_str(), Utils::doubleMaxString(order.cashQty).c_str(), decimalStringToDisplay(order.filledQuantity).c_str(),
        Utils::doubleMaxString(order.lmtPrice).c_str(), Utils::doubleMaxString(order.auxPrice).c_str(), orderState.status.c_str(), orderState.completedTime.c_str(), orderState.completedStatus.c_str(),
        Utils::intMaxString(order.minTradeQty).c_str(), Utils::intMaxString(order.minCompeteSize).c_str(),
        order.competeAgainstBestOffset == COMPETE_AGAINST_BEST_OFFSET_UP_TO_MID ? "UpToMid" : Utils::doubleMaxString(order.competeAgainstBestOffset).c_str(),
        Utils::doubleMaxString(order.midOffsetAtWhole).c_str(), Utils::doubleMaxString(order.midOffsetAtHalf).c_str());
}

void Mlogger::position(const std::string& account, const Contract& contract, Decimal position, double avgCost) {
	fprintf(lfp, "Position. %s - Symbol: %s, SecType: %s, Currency: %s, Position: %s, Avg Cost: %s\n", 
		account.c_str(), contract.symbol.c_str(), contract.secType.c_str(), 
		contract.currency.c_str(), decimalStringToDisplay(position).c_str(), 
		Utils::doubleMaxString(avgCost).c_str());
}

void Mlogger::execDetails( int reqId, const Contract& contract, const Execution& execution) {
	fprintf(lfp, "ExecDetails. ReqId: %d - %s, %s, %s - %s, %s, %s, %s, %s\n", reqId, contract.symbol.c_str(), contract.secType.c_str(), contract.currency.c_str(), execution.execId.c_str(), Utils::longMaxString(execution.orderId).c_str(), decimalStringToDisplay(execution.shares).c_str(), decimalStringToDisplay(execution.cumQty).c_str(), Utils::intMaxString(execution.lastLiquidity).c_str());
}



/**** PRIVATE METHODS ****/

void Mlogger::printContractMsg(const Contract& contract) {
    fprintf(lfp, "\tConId: %ld\n", contract.conId);
	fprintf(lfp, "\tSymbol: %s\n", contract.symbol.c_str());
	fprintf(lfp, "\tSecType: %s\n", contract.secType.c_str());
	fprintf(lfp, "\tLastTradeDateOrContractMonth: %s\n", contract.lastTradeDateOrContractMonth.c_str());
	fprintf(lfp, "\tStrike: %s\n", Utils::doubleMaxString(contract.strike).c_str());
	fprintf(lfp, "\tRight: %s\n", contract.right.c_str());
	fprintf(lfp, "\tMultiplier: %s\n", contract.multiplier.c_str());
	fprintf(lfp, "\tExchange: %s\n", contract.exchange.c_str());
	fprintf(lfp, "\tPrimaryExchange: %s\n", contract.primaryExchange.c_str());
	fprintf(lfp, "\tCurrency: %s\n", contract.currency.c_str());
	fprintf(lfp, "\tLocalSymbol: %s\n", contract.localSymbol.c_str());
	fprintf(lfp, "\tTradingClass: %s\n", contract.tradingClass.c_str());
}

void Mlogger::printContractDetailsSecIdList(const TagValueListSPtr &secIdList) {
    const int secIdListCount = secIdList.get() ? secIdList->size() : 0;
	if (secIdListCount > 0) {
		fprintf(lfp, "\tSecIdList: {");
		for (int i = 0; i < secIdListCount; ++i) {
			const TagValue* tagValue = ((*secIdList)[i]).get();
			fprintf(lfp, "%s=%s;",tagValue->tag.c_str(), tagValue->value.c_str());
		}
		fprintf(lfp, "}\n");
	}
}

void Mlogger::printContractDetailsMsg(const ContractDetails& contractDetails) {
    fprintf(lfp, "\tMarketName: %s\n", contractDetails.marketName.c_str());
	fprintf(lfp, "\tMinTick: %s\n", Utils::doubleMaxString(contractDetails.minTick).c_str());
	fprintf(lfp, "\tPriceMagnifier: %s\n", Utils::longMaxString(contractDetails.priceMagnifier).c_str());
	fprintf(lfp, "\tOrderTypes: %s\n", contractDetails.orderTypes.c_str());
	fprintf(lfp, "\tValidExchanges: %s\n", contractDetails.validExchanges.c_str());
	fprintf(lfp, "\tUnderConId: %s\n", Utils::intMaxString(contractDetails.underConId).c_str());
	fprintf(lfp, "\tLongName: %s\n", contractDetails.longName.c_str());
	fprintf(lfp, "\tContractMonth: %s\n", contractDetails.contractMonth.c_str());
	fprintf(lfp, "\tIndystry: %s\n", contractDetails.industry.c_str());
	fprintf(lfp, "\tCategory: %s\n", contractDetails.category.c_str());
	fprintf(lfp, "\tSubCategory: %s\n", contractDetails.subcategory.c_str());
	fprintf(lfp, "\tTimeZoneId: %s\n", contractDetails.timeZoneId.c_str());
	fprintf(lfp, "\tTradingHours: %s\n", contractDetails.tradingHours.c_str());
	fprintf(lfp, "\tLiquidHours: %s\n", contractDetails.liquidHours.c_str());
	fprintf(lfp, "\tEvRule: %s\n", contractDetails.evRule.c_str());
	fprintf(lfp, "\tEvMultiplier: %s\n", Utils::doubleMaxString(contractDetails.evMultiplier).c_str());
	fprintf(lfp, "\tAggGroup: %s\n", Utils::intMaxString(contractDetails.aggGroup).c_str());
	fprintf(lfp, "\tUnderSymbol: %s\n", contractDetails.underSymbol.c_str());
	fprintf(lfp, "\tUnderSecType: %s\n", contractDetails.underSecType.c_str());
	fprintf(lfp, "\tMarketRuleIds: %s\n", contractDetails.marketRuleIds.c_str());
	fprintf(lfp, "\tRealExpirationDate: %s\n", contractDetails.realExpirationDate.c_str());
	fprintf(lfp, "\tLastTradeTime: %s\n", contractDetails.lastTradeTime.c_str());
	fprintf(lfp, "\tStockType: %s\n", contractDetails.stockType.c_str());
	fprintf(lfp, "\tMinSize: %s\n", decimalStringToDisplay(contractDetails.minSize).c_str());
	fprintf(lfp, "\tSizeIncrement: %s\n", decimalStringToDisplay(contractDetails.sizeIncrement).c_str());
	fprintf(lfp, "\tSuggestedSizeIncrement: %s\n", decimalStringToDisplay(contractDetails.suggestedSizeIncrement).c_str());
    printContractDetailsSecIdList(contractDetails.secIdList); 
}

void Mlogger::printBondContractDetailsMsg(const ContractDetails& contractDetails) {
    fprintf(lfp, "\tSymbol: %s\n", contractDetails.contract.symbol.c_str());
	fprintf(lfp, "\tSecType: %s\n", contractDetails.contract.secType.c_str());
	fprintf(lfp, "\tCusip: %s\n", contractDetails.cusip.c_str());
	fprintf(lfp, "\tCoupon: %s\n", Utils::doubleMaxString(contractDetails.coupon).c_str());
	fprintf(lfp, "\tMaturity: %s\n", contractDetails.maturity.c_str());
	fprintf(lfp, "\tIssueDate: %s\n", contractDetails.issueDate.c_str());
	fprintf(lfp, "\tRatings: %s\n", contractDetails.ratings.c_str());
	fprintf(lfp, "\tBondType: %s\n", contractDetails.bondType.c_str());
	fprintf(lfp, "\tCouponType: %s\n", contractDetails.couponType.c_str());
	fprintf(lfp, "\tConvertible: %s\n", contractDetails.convertible ? "yes" : "no");
	fprintf(lfp, "\tCallable: %s\n", contractDetails.callable ? "yes" : "no");
	fprintf(lfp, "\tPutable: %s\n", contractDetails.putable ? "yes" : "no");
	fprintf(lfp, "\tDescAppend: %s\n", contractDetails.descAppend.c_str());
	fprintf(lfp, "\tExchange: %s\n", contractDetails.contract.exchange.c_str());
	fprintf(lfp, "\tCurrency: %s\n", contractDetails.contract.currency.c_str());
	fprintf(lfp, "\tMarketName: %s\n", contractDetails.marketName.c_str());
	fprintf(lfp, "\tTradingClass: %s\n", contractDetails.contract.tradingClass.c_str());
	fprintf(lfp, "\tConId: %s\n", Utils::longMaxString(contractDetails.contract.conId).c_str());
	fprintf(lfp, "\tMinTick: %s\n", Utils::doubleMaxString(contractDetails.minTick).c_str());
	fprintf(lfp, "\tOrderTypes: %s\n", contractDetails.orderTypes.c_str());
	fprintf(lfp, "\tValidExchanges: %s\n", contractDetails.validExchanges.c_str());
	fprintf(lfp, "\tNextOptionDate: %s\n", contractDetails.nextOptionDate.c_str());
	fprintf(lfp, "\tNextOptionType: %s\n", contractDetails.nextOptionType.c_str());
	fprintf(lfp, "\tNextOptionPartial: %s\n", contractDetails.nextOptionPartial ? "yes" : "no");
	fprintf(lfp, "\tNotes: %s\n", contractDetails.notes.c_str());
	fprintf(lfp, "\tLong Name: %s\n", contractDetails.longName.c_str());
	fprintf(lfp, "\tEvRule: %s\n", contractDetails.evRule.c_str());
	fprintf(lfp, "\tEvMultiplier: %s\n", Utils::doubleMaxString(contractDetails.evMultiplier).c_str());
	fprintf(lfp, "\tAggGroup: %s\n", Utils::intMaxString(contractDetails.aggGroup).c_str());
	fprintf(lfp, "\tMarketRuleIds: %s\n", contractDetails.marketRuleIds.c_str());
	fprintf(lfp, "\tTimeZoneId: %s\n", contractDetails.timeZoneId.c_str());
	fprintf(lfp, "\tLastTradeTime: %s\n", contractDetails.lastTradeTime.c_str());
	fprintf(lfp, "\tMinSize: %s\n", decimalStringToDisplay(contractDetails.minSize).c_str());
	fprintf(lfp, "\tSizeIncrement: %s\n", decimalStringToDisplay(contractDetails.sizeIncrement).c_str());
	fprintf(lfp, "\tSuggestedSizeIncrement: %s\n", decimalStringToDisplay(contractDetails.suggestedSizeIncrement).c_str());
	printContractDetailsSecIdList(contractDetails.secIdList);
}
