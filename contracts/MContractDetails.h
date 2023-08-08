
#pragma once
#include "Contract.h"

class MContractDetails
{
    public:

    static ContractDetails DAXInd();
    static ContractDetails DAXFut();
    static ContractDetails NIKInd();
    static ContractDetails NIKFut();

    // TEST
    static ContractDetails CryptoContract();
    static ContractDetails EurGbpFx();

    static ContractDetails USBond();
    static ContractDetails USStockCFD();
    static ContractDetails IBMStockAtSmart();
    static ContractDetails AMZNStockAtSmart();


};