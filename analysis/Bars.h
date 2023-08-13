
#pragma once

#include <string>
#include <typeinfo>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Bar.h"
#include "../myclient/CommonMacros.h"

class Bars
{
    int numBars = 0;
    MBar * barArray [MAXBARS];

    /****** FILE-PARSING ********/
    int parseFile(const char* inputFileDir, const char* startDate, const char* endDate);
   
    public:
    /*** PUBLIC INTERFACE ***/
    Bars(const char* inputFileDir = NULL, 
         const char* startDate    = NULL, 
         const char* endDate      = NULL);
    ~Bars();
    void addBar(MBar*);
    void printBars(const std::string outputDir, const std::string name = "", const std::string outputExt = ".txt");
    static void clear_file(const char* filename);

    /*** GETTERS & SETTERS ***/
    MBar*       getBar(int i)  const { return this->barArray[i];}
    int         getnumBars()   const { return numBars;}

    private:
    enum Data {
        DATE_TIME,
        OPEN,
        CLOSE,
        HIGH,
        LOW,
        VOL,
        LOC_DATE_TIME,
        NUM_DATA
    };
    const std::string DataStr[NUM_DATA] = 
        {"dateTime", "open", "close", "high", "low", "vol", "locDateTime"};
    
};
