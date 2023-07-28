
#pragma once

#include "Contract.h"
#include "CommonDefs.h"
#include "Decimal.h"
#include "Bars.h"
#include "bar.h"
#include "StdAfx.h"
#include <time.h>

#include "Mlogger.h"

#define MAXBARSIZESTRINGLENGTH 8

class Instrument
{
    Mlogger* const m_logger;
    long last_bar_update;
    const int cross_validation_bars = 8;

    int parse_barSize( const std::string barSize);
    static long curr_sys_time();

    public:

    const int inst_id;
    const std::string barSize;
    const int sec_barSize;
    int bars2update = 0;
    char* extend_dur(int factor);
    bool requires_update();

    Bars* bars;
    ContractDetails dataContract;
    ContractDetails orderContract;

    bool addBar(TickerId reqId, const Bar& bar, const double time_tol = 1.5, const int update_factor = 2);                             // passed in historicalDataUpdate callback
    void updateDataContract (int reqId, const ContractDetails& contractDetails); // passed in contractDetails callback
    void updateOrderContract(int reqId, const ContractDetails& contractDetails); // passed in contractDetails callback

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

    ReqIds m_reqIds;

    Instrument(const int inst_id, 
               const std::string barSize, 
               ContractDetails dataContract, 
               ContractDetails orderContract, 
               ReqIds reqIds, 
               std::string logFilePath = "./Instrument_log.txt");
    ~Instrument();

};
