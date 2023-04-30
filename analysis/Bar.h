
#ifndef __BAR_H
#define __BAR_H

#include <ctime>
#include <stdlib.h>
#include <string>

typedef struct tm date_t;

class Bar
{
    const int maxDateChar = 20;
    date_t* Date_Time;
    const float Open, Close, High, Low, Vol;

    public:

    Bar(float, float, float, float, float = 0.0f, 
        date_t* = NULL, char* = NULL);
    char* date_time_str;
    void Delete();

    /*** GETTERS & SETTERS ***/
    date_t* date_time() const { return Date_Time; }
    float open() const { return Open; }
    float close() const { return Close; }
    float high() const { return High; }
    float low() const { return Low; }
    float vol() const { return Vol; }
};


#endif //__BAR_H