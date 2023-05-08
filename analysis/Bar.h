
#pragma once

#include <ctime>
#include <stdlib.h>
#include <string>

typedef struct tm date_t;

class MBar
{
    const double      Open, Close, High, Low, Vol;
    const int         maxDateChar = 20;
    /*const std::string Time_zone; */
    date_t*           Date_Time;

    public:

    MBar(double, double, double, double, double = 0.0f, 
        date_t* = NULL, /*std::string = "",*/ char* = NULL);
    ~MBar();
    char* date_time_str;
    
    /*** GETTERS & SETTERS ***/
    double      open()      const { return Open;}
    double      close()     const { return Close;}
    double      high()      const { return High;}
    double      low()       const { return Low;}
    double      vol()       const { return Vol;}
    date_t*     date_time() const { return Date_Time;}
    /*std::string time_zone() const { return Time_zone;} */
};
