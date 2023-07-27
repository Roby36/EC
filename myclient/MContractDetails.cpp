
#include "MContractDetails.h"

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
    contractDetails.contract.symbol   = "NIKKEI";
	contractDetails.contract.secType  = "IND";
	contractDetails.contract.currency = "JPY";
	contractDetails.contract.exchange = "TSE";
	return contractDetails;
}

ContractDetails MContractDetails::NIKFut() {
    ContractDetails contractDetails;
    contractDetails.contract.symbol 	= "NIKKEI";
	contractDetails.contract.secType    = "FUT";
	contractDetails.contract.currency   = "JPY";
	contractDetails.contract.exchange   = "TSE";
    contractDetails.contract.multiplier = "1";
	return contractDetails;
}

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
