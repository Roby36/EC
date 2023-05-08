
#pragma once

#include "Contract.h"
#include "CommonDefs.h"
#include "Decimal.h"
#include "Bars.h"
#include "bar.h"
#include "StdAfx.h"
#include <time.h>

#include "Mlogger.h"

class Instrument
{
    Mlogger* const m_logger;

    public:

    /*** Important: all reqIds must be different across Instruments ****/
    typedef struct {
        int orderContract;
        int dataContract;
        int realTimeBars;
        TickerId historicalBars;
        TickerId updatedBars;
        
    } ReqIds;

    Instrument(const int inst_id, const std::string barSize, ContractDetails dataContract, 
        ContractDetails orderContract, ReqIds reqIds, std::string logFilePath = "./Instrument_log.txt");
    ~Instrument();

    const int inst_id;
    const std::string barSize;

    /*** DEFENSIVE PARAMETERS ****/
    const int cross_validation_bars = 8; // number of bars to cross-validate against when updating bars

    ReqIds m_reqIds;

    Bars* bars;
    ContractDetails dataContract;
    ContractDetails orderContract;

    void addBar(TickerId reqId, const Bar& bar);                             // passed in historicalDataUpdate callback
    void updateDataContract (int reqId, const ContractDetails& contractDetails); // passed in contractDetails callback
    void updateOrderContract(int reqId, const ContractDetails& contractDetails); // passed in contractDetails callback

    // Function to fill up the struct upon initialization
    static ReqIds reqIds(int orderContract, int dataContract, 
                    int realTimeBars, TickerId historicalBars, TickerId updatedBars);

};