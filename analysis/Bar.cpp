
#include "Bar.h"

MBar::MBar(double open, double close, double high, double low, double vol, 
    date_t* date_time, /*std::string Time_zone,*/ char* date_time_str)
    : Open(open), Close(close), High(high), Low(low), Vol(vol), 
        Date_Time(date_time), /*Time_zone(Time_zone),*/ date_time_str(date_time_str)
{
    this->Date_Time = (date_t*) malloc(sizeof(date_t));
    if (date_time != NULL) {
        *this->Date_Time = *date_time;
    }
    this->date_time_str = (char*) malloc(this->maxDateChar);
    if (date_time_str != NULL) {
        strcpy(this->date_time_str, date_time_str);
    }
}

MBar::~MBar()
{
    if (this->date_time_str != NULL) { free(this->date_time_str);}
    if (this->date_time_str != NULL) { free(this->Date_Time);}
}