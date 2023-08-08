
#include "Instrument.h"

Instrument::Instrument(const int inst_id, 
                       const std::string barSize, 
                       ContractDetails dataContract, 
                       ContractDetails orderContract, 
                       ReqIds reqIds, 
                       std::string logFilePath)
                        : m_logger(new Mlogger( logFilePath)),  
                          inst_id(inst_id), 
                          barSize(barSize), 
                          sec_barSize( parse_barSize( barSize)),
                          dataContract(dataContract), 
                          orderContract(orderContract),
                          m_reqIds(reqIds)                   
{
    // Check if bar size successfully parsed 
    if (sec_barSize == -1) {
        m_logger->str( "Could not initialize instrument " + std::to_string( inst_id)
                        + " due to invalid barSize. Exiting program.\n");
        exit(7);
    }
    //Initialize instrument's Bars object:
    this->bars = new Bars();
}

Instrument::~Instrument()
{
    //TO-DO: determine what to free
    delete(this->bars);
    delete m_logger;
}

int Instrument::parse_barSize( const std::string barSize)
{
    // Attemp to extract time units
    int n;
    char str[MAX_BARSIZE_STRING_LENGTH];
    if ( sscanf( barSize.c_str(), "%d %s", &n, str) != 2) {
        m_logger->str( "Error processing barSize: invalid string\n");
        return -1;
    }
    // Compute barSize in seconds
    switch (char f = str[0]) {
        case 's': return n;
        case 'm': return n * 60;
        case 'h': return n * 60 * 60;
        case 'd': return n * 60 * 60 * 24;
        case 'w': return n * 60 * 60 * 24 * 7;
        default: {
            m_logger->str( "Error processing barSize: invalid time units\n");
            return -1;
        }
    }
}

/*** utility function: MALLOC'D STRING MUST BE FREED ***/
char* Instrument::extend_dur(int factor)
{
	// Tokenize string
	char cstr[MAX_BARSIZE_STRING_LENGTH];
	int dur;
  	if (sscanf(barSize.c_str(), "%d %s", &dur, cstr) != 2) {
		m_logger->str( "\textend_dur() error: invalid barSize string\n");
		return NULL;
	}	
	// convert barsize settings to a duration string
	int  tf = 1;
	char tc = 'S';
	switch (char t_unit = cstr[0]) {
		case 's': 				 break;					   
		case 'm': tf = 60;	     break;
		case 'h': tf = 60 * 60;  break;
		case 'd': tc = 'D'; 	 break;
		default:
			m_logger->str( "\textend_dur() error: unsupported barSize\n");
			return NULL;
	}
    // Check that we are not exceeding the maximum seconds for data request
    int tot_time_units =  tf * factor * dur;
    if (tc == 'S' && tot_time_units >= MAX_TOTAL_SECONDS) {
        tc = 'D'; // change character to days, and decrement total time units accordingly
        tot_time_units = tot_time_units / (60 * 60 * 24); /** NOTE: not significant loss of precision here */
    }
    if (tc == 'D' && tot_time_units >= MAX_TOTAL_DAYS) {
        tc = 'Y'; 
        tot_time_units = tot_time_units / 365;
    }
	// Construct output string
	char buff[MAX_BARSIZE_STRING_LENGTH];
	int char_written = snprintf( buff, MAX_BARSIZE_STRING_LENGTH, "%d %c", tot_time_units, tc);
	m_logger->str( "\textend_dur(): snprintf() recorded " + std::to_string(char_written) + " characters\n");
	// Handle exception
	if (char_written < 0) {
		m_logger->str( "\textend_dur(): snprintf() failed\n");
		return NULL;
	}
	// Allocate the correct amount of memory to fully free string later
	char* out = new char[char_written + 1]; // include terminating null character
	strncpy( out, buff,  char_written + 1);
	// Log output
	m_logger->str( "\textend_dur(" + barSize + ") = " + std::string(out) + "\n");
	return out;	
}

bool Instrument::requires_update() {
    return ((int)(curr_sys_time() - last_bar_update) >= sec_barSize);
}

long Instrument::curr_sys_time() {
    return (long) std::time(NULL);
}

bool Instrument::addBar(TickerId reqId, const Bar& bar, const double time_tol, const int update_factor) {
    // Ignore call if reqId does not correspond with any valid option
    if (this->m_reqIds.historicalBars != reqId
     && this->m_reqIds.updatedBars    != reqId)
        return false;
    struct tm timeinfo;
    // Time zones not important for daily bars
    if (this->barSize == "1 day")
        strptime((bar.time + std::string(" 00:00:00")).c_str(), "%Y%m%d %H:%M:%S", &timeinfo);
    else
        strptime(bar.time.c_str(), "%Y%m%d %H:%M:%S %Z", &timeinfo);
    //! Add other possible bar sizes above?
    // Compare time with previous bars' time
    int numBars = bars->getnumBars();
    for (int bars_back = 0; 
             bars_back < std::min(cross_validation_bars, numBars); 
             bars_back++) {
        tm* prev_date = bars->getBar((numBars - 1) - bars_back)->date_time();
#ifdef DATEDBG
    char curr_time_str[32];
    char prev_time_str[32];
    strftime(curr_time_str, 32, "%Y%m%d %H:%M:%S %Z", &timeinfo);
    strftime(prev_time_str, 32, "%Y%m%d %H:%M:%S %Z", prev_date);
#endif
        time_t curr_time = mktime(&timeinfo);
        time_t prev_time = mktime(prev_date);
        double time_diff = difftime(curr_time, prev_time);
        if (time_diff < (double) sec_barSize) {
            // Log error and return
            m_logger->str("\tCannot add bar for instrument " + std::to_string(inst_id)
                            + " with date "                  + asctime(&timeinfo) 
                            + ": most recent bar found on "  + asctime(prev_date) +"\n");
            return false;
        } 
        /** NOTE: This check is hard to implement due to trading hours irregularities! **/
        else if (bars_back == 0 && 
                 time_diff > (double) (time_tol * sec_barSize) &&
                 within_trading_hours(this->dataContract)) /* check ignored if not within trading hours */ {
            bars2update = (update_factor * time_diff) / sec_barSize;
            // Request bar update
            m_logger->str("\tCannot add bar for instrument "  + std::to_string(inst_id)
                            + " with date "                   + asctime(&timeinfo) 
                            + ": missing bars in between "    + asctime(prev_date) 
                            + " and requesting an update of " + std::to_string(bars2update)
                            + " bars.\n");
            // Determine how many bars are required (MClient responsible for setting bars2update back to zero!)
            return false;
        }
    }
    // Construct new Bar object
    MBar* n_bar = new MBar(bar.open, bar.close, bar.high, bar.low,
                         decimalToDouble(bar.volume), (date_t*) &timeinfo);
    // Insert new Bar
    this->bars->addBar(n_bar);
    // Record time of insertion
    this->last_bar_update = curr_sys_time();
    // Log new bar's insertion
    m_logger->str("\tSuccessfully added bar " + std::to_string(this->bars->getnumBars() - 1) + 
                  " for instrument " + std::to_string(inst_id) +
                  " with date "      + asctime(&timeinfo) +
                  " at system time " + std::to_string(last_bar_update) + "\n");
    return true;
}         

bool Instrument::within_trading_hours(ContractDetails contract_details) 
{
    /** EXAMPLES:
    20230806:CLOSED;20230807:0215-20230807:2205;20230808:0215-20230808:2205;20230809:0215-20230809:2205;20230810:0215-20230810:2205;20230811:0215-20230811:2205
    20230806:CLOSED;
    20230807:0215 - 20230807:2205 ;
    */
    char th_string [MAX_TH_STRING_LENGTH]; // Copy constant date-time string into modifiable array
    strncpy(th_string, contract_details.tradingHours.c_str(), MAX_TH_STRING_LENGTH);
    char * th_string_it = th_string;
    while (*(++th_string_it) != ';') {} // iterate until the first ';' 
    *th_string_it = '\0'; // truncate string
    // Parse string contents into opening and closing time strings
    char opening_time_str[MAX_TH_STRING_LENGTH / 2];
    char closing_time_str[MAX_TH_STRING_LENGTH / 2];
    if (sscanf(th_string, "%s-%s", opening_time_str, closing_time_str) != 2)
        return false; // this implies that market is closed today
    // Parse strings into time structs
    struct tm opening_time_struct;
    struct tm closing_time_struct;
    strptime(opening_time_str, "%Y%m%d:%H%M", &opening_time_struct);
    strptime(closing_time_str, "%Y%m%d:%H%M", &closing_time_struct);
    // Compare time structs with current time 
    time_t opening_time = mktime(&opening_time_struct);
    time_t closing_time = mktime(&closing_time_struct);
    time_t curr_time    = std::time(NULL);
    if (difftime(curr_time, opening_time) < 0 ||
        difftime(curr_time, closing_time) > 0)
        return false;
    return true;
}

void Instrument::updateDataContract (int reqId, const ContractDetails& contractDetails) {
    // Ignore call if reqId does not correspond
    if (this->m_reqIds.dataContract != reqId)
        return;
    this->dataContract = contractDetails; // Update contract
}

void Instrument::updateOrderContract(int reqId, const ContractDetails& contractDetails) {
    // Ignore call if reqId does not correspond
    if (this->m_reqIds.orderContract != reqId) 
        return;
    this->orderContract = contractDetails; // Update contract
}
