
#include "Bar.h"

MBar::MBar(double open, 
           double close, 
           double high, 
           double low, 
           double vol, 
           date_t* date_time,
           char* date_time_str)
    : Open(open), 
      Close(close), 
      High(high), 
      Low(low), 
      Vol(vol) 
{
    // Malloc date_t pointer
    if (date_time != NULL) {
        this->Date_Time = (date_t*) malloc (sizeof(date_t));
        memcpy(this->Date_Time, date_time, sizeof(date_t));
        //*this->Date_Time = *date_time;
    } else {
        this->Date_Time = NULL;
    }
    this->date_time_str = (char*) malloc (this->maxDateChar);
    // enter date-time string if directly provided
    if (date_time_str != NULL) {
        strcpy(this->date_time_str, date_time_str);
    // Otherwise parse the date_t object given if valid
    } else if (date_time != NULL)  {
        strftime(this->date_time_str, maxDateChar, "%d/%m/%Y %H:%M:%S %Z", date_time);
    } else {
        free(this->date_time_str);
        this->date_time_str = NULL;
    }
}

MBar::~MBar()
{
    if (this->date_time_str != NULL) 
        free(this->date_time_str);
    if (this->Date_Time != NULL) 
        free(this->Date_Time);
}


