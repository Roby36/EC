
#pragma once

#include <ctime>
#include <stdlib.h>
#include <string>
#include <time.h>

typedef struct tm date_t;

class MBar
{
    const double Open, Close, High, Low, Vol;
    const int    maxDateChar = 32;
    date_t*      Date_Time;

    public:

    MBar(double open, 
         double close, 
         double high, 
         double low,  
         double vol = 0.0f, 
         date_t* date_time   = NULL,
         char* date_time_str = NULL);
    ~MBar();
    char* date_time_str;
    
    /*** GETTERS & SETTERS ***/
    double      open()      const { return Open;}
    double      close()     const { return Close;}
    double      high()      const { return High;}
    double      low()       const { return Low;}
    double      vol()       const { return Vol;}
    date_t*     date_time() const { return Date_Time;}
};
