
#include "Instrument.h"

Instrument::Instrument(const int inst_id, const std::string barSize, ContractDetails dataContract, 
        ContractDetails orderContract, ReqIds reqIds, std::string logFilePath)
         : inst_id(inst_id), barSize(barSize), dataContract(dataContract), orderContract(orderContract),
            m_reqIds(reqIds), m_logger(new Mlogger( logFilePath))
{
    //Initialize instrument's Bars object:
    this->bars = new Bars();
}

Instrument::~Instrument()
{
    //TO-DO: determine what to free
    delete(this->bars);
    delete m_logger;
}

void Instrument::addBar(TickerId reqId, const Bar& bar) {
    // Ignore call if reqId does not correspond
    if (this->m_reqIds.historicalBars != reqId) {
        return;
    }
    struct tm timeinfo;
    // Time zones not important for daily bars
    if (this->barSize == "1 day") {
        strptime(bar.time.c_str(), "%Y%m%d", &timeinfo);
    } else if (this->barSize == "1 hour") {
        strptime(bar.time.c_str(), "%Y%m%d %H:%M:%S %Z", &timeinfo);
    }
    // Compare time with previous bars' time
    int numBars = bars->getnumBars();
    for (int bars_back = 0; 
             bars_back < std::min(cross_validation_bars, numBars); 
             bars_back++) {
        tm* prev_date = bars->getBar((numBars - 1) - bars_back)->date_time();
        if ( difftime( mktime(prev_date), mktime(&timeinfo)) > 0) {
            // Log error and return
            m_logger->str("\tCannot add bar for instrument " + to_string(inst_id)
                            + " with date " + asctime(&timeinfo) + "\n");
            return;
        }
    }
    // Construct new Bar object
    MBar* n_bar = new MBar(bar.open, bar.close, bar.high, bar.low,
                         decimalToDouble(bar.volume), (date_t*) &timeinfo);
    // Insert new Bar
    this->bars->addBar(n_bar);
    // Log new bar's insertion
    m_logger->str("\tSuccessfully added bar for instrument " + to_string(inst_id)
                            + " with date " + asctime(&timeinfo) + "\n");
}         

void Instrument::updateDataContract (int reqId, const ContractDetails& contractDetails) {
    // Ignore call if reqId does not correspond
    if (this->m_reqIds.dataContract != reqId) {
        return;
    }
    // Update contract
    this->dataContract = contractDetails;
}

void Instrument::updateOrderContract(int reqId, const ContractDetails& contractDetails) {
    // Ignore call if reqId does not correspond
    if (this->m_reqIds.orderContract != reqId) {
        return;
    }
    // Update contract
    this->orderContract = contractDetails;
}

Instrument::ReqIds Instrument::reqIds(int orderContract, int dataContract, 
                    int realTimeBars, TickerId historicalBars, TickerId updatedBars) {
    Instrument::ReqIds reqIds;
    reqIds.orderContract  = orderContract;
    reqIds.dataContract   = dataContract;
    reqIds.realTimeBars   = realTimeBars;
    reqIds.historicalBars = historicalBars;
    reqIds.updatedBars    = updatedBars;
    return reqIds;
}

