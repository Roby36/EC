
#include "MContractDetails.h"

/*
ContractDetails MContractDetails:: {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = 
	contractDetails.contract.secType  = 
	contractDetails.contract.currency = 
	contractDetails.contract.exchange = 
	return contractDetails;
}
*/

ContractDetails MContractDetails::DAXInd() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = "DAX";
	contractDetails.contract.secType  = "IND";
	contractDetails.contract.currency = "EUR";
	contractDetails.contract.exchange = "EUREX";
	return contractDetails;
}

ContractDetails MContractDetails::DAXFut() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol     = "DAX";
	contractDetails.contract.secType    = "FUT";
	contractDetails.contract.currency   = "EUR";
	contractDetails.contract.exchange   = "EUREX";
    contractDetails.contract.multiplier = "1";
	return contractDetails;
}

ContractDetails MContractDetails::NIKInd() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = "N225";
	contractDetails.contract.secType  = "IND";
	contractDetails.contract.currency = "JPY";
	contractDetails.contract.exchange = "OSE.JPN";
	return contractDetails;
}

ContractDetails MContractDetails::NIKFut() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol 	= "N225";
	contractDetails.contract.secType    = "FUT";
	contractDetails.contract.currency   = "JPY";
	contractDetails.contract.exchange   = "OSE.JPN";
    contractDetails.contract.multiplier = "1000";
	return contractDetails;
}

// TEST

ContractDetails MContractDetails::CryptoContract() {
	ContractDetails contractDetails;
	contractDetails.contract.symbol   = "BTC";
	contractDetails.contract.secType  = "CRYPTO";
	contractDetails.contract.exchange = "PAXOS";
	contractDetails.contract.currency = "USD";
	return contractDetails;
}

ContractDetails MContractDetails::EurGbpFx(){
	ContractDetails contractDetails;
	contractDetails.contract.symbol = "EUR";
	contractDetails.contract.secType = "CASH";
	contractDetails.contract.currency = "GBP";
	contractDetails.contract.exchange = "IDEALPRO";
	return contractDetails;
}

ContractDetails MContractDetails::USBond() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = "IBM";
	contractDetails.contract.secType  = "BOND";
	contractDetails.contract.currency = "USD";
	contractDetails.contract.exchange = "SMART";
	return contractDetails;
}

ContractDetails MContractDetails::USStockCFD() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = "IBM";
	contractDetails.contract.secType  = "CFD";
	contractDetails.contract.currency = "USD";
	contractDetails.contract.exchange = "SMART";
	return contractDetails;
}

ContractDetails MContractDetails::IBMStockAtSmart() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = "IBM";
	contractDetails.contract.secType  = "STK";
	contractDetails.contract.currency = "USD";
	contractDetails.contract.exchange = "SMART";
	return contractDetails;
}

ContractDetails MContractDetails::AMZNStockAtSmart() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = "AMZN";
	contractDetails.contract.secType  = "STK";
	contractDetails.contract.currency = "USD";
	contractDetails.contract.exchange = "SMART";
	return contractDetails;
}

ContractDetails MContractDetails::ESTX50Index() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = "ESTX50";
	contractDetails.contract.secType  = "IND";
	contractDetails.contract.currency = "EUR";
	contractDetails.contract.exchange = "EUREX";
	return contractDetails;
}

ContractDetails MContractDetails::ESTX50Future() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol   = "ESTX50";
	contractDetails.contract.secType  = "FUT";
	contractDetails.contract.currency = "EUR";
	contractDetails.contract.exchange = "EUREX";
	contractDetails.contract.multiplier = "1";
	return contractDetails;
}



