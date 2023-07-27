
#pragma once

#include <string>
#include <typeinfo>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Bar.h"

#define MAXBARS 65536   //2^16

class Bars
{
    int numBars = 0;
    MBar * barArray [MAXBARS];

    /****** FILE-PARSING ********/
    const std::string outputDir = "../data/";
    const std::string outputExt = ".txt";
    int parseFile(const char* inputFileDir, const char* startDate, const char* endDate);
   
    public:

    /*** PUBLIC INTERFACE ***/
    Bars(const char* inputFileDir = NULL, 
         const char* startDate    = NULL, 
         const char* endDate      = NULL);
    ~Bars();
    void addBar(MBar*);
    void printBars();

    /*** GETTERS & SETTERS ***/
    MBar*       getBar(int i)  const { return this->barArray[i];}
    int         getnumBars()   const { return numBars;}
    std::string getoutputDir() const { return outputDir;}
    std::string getoutputExt() const { return outputExt;}
};
