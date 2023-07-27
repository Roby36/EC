
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
    Mlogger* /*const*/ m_logger;
    int parse_barSize( const std::string barSize);

    public:

    /*** Important: all reqIds must be different across Instruments ****/
    typedef struct RI {

        int orderContract;
        int dataContract;
        int realTimeBars;
        TickerId historicalBars;
        TickerId updatedBars;

        RI(const int orderContract, 
           const int dataContract, 
           const int realTimeBars, 
           const TickerId historicalBars, 
           const TickerId updatedBars)
            : orderContract(orderContract),
              dataContract(dataContract),
              realTimeBars(realTimeBars),
              historicalBars(historicalBars),
              updatedBars(updatedBars)
        {
        }
        
    } ReqIds;

    Instrument(const int inst_id, 
               const std::string barSize, 
               ContractDetails dataContract, 
               ContractDetails orderContract, 
               ReqIds reqIds, 
               std::string logFilePath = "./Instrument_log.txt");
    ~Instrument();

    /*const*/ int inst_id;
    /*const*/ std::string barSize;
    /*const*/ int sec_barSize;

    //** System time for last bar update (seconds) **//
    long last_bar_update;

    /*** DEFENSIVE PARAMETERS ****/
    /*const*/ int cross_validation_bars = 8; // number of bars to cross-validate against when updating bars

    ReqIds m_reqIds;

    Bars* bars;
    ContractDetails dataContract;
    ContractDetails orderContract;

    bool addBar(TickerId reqId, const Bar& bar);                             // passed in historicalDataUpdate callback
    void updateDataContract (int reqId, const ContractDetails& contractDetails); // passed in contractDetails callback
    void updateOrderContract(int reqId, const ContractDetails& contractDetails); // passed in contractDetails callback

    // Function to get current system time (to regulate updates)
    static long curr_sys_time();
};
