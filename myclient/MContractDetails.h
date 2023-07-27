
#pragma once
#include "Contract.h"

class MContractDetails
{
    public:

    static ContractDetails DAXInd();
    static ContractDetails DAXFut();
    static ContractDetails NIKInd();
    static ContractDetails NIKFut();

    static ContractDetails CryptoContract();
    static ContractDetails EurGbpFx();

};
