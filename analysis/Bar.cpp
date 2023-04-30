
#include "Bar.h"

Bar::Bar(float open, float close, float high, float low, float vol, date_t* date_time, char* date_time_str)
    : Open(open), Close(close), High(high), Low(low), Vol(vol), Date_Time(date_time), date_time_str(date_time_str)
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

void Bar::Delete()
{
    if (this->date_time_str != NULL) { free(this->date_time_str); }
    if (this->date_time_str != NULL) { free(this->Date_Time); }
    delete(this);
}