
#pragma once

#include <string>
#include <typeinfo>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Bar.h"

using namespace std;

class Bars
{
    /*const int TimePeriod; // minutes */
    const int maxBars;
    int numBars = 0;
    MBar** barArray;

    /****** FILE-PARSING ********/
    const string outputDir = "../data/";
    const string outputExt = ".txt";
    int parseFile(const char*, const char*, const char*);

    public:

    /*** PUBLIC INTERFACE ***/
    Bars(const char* = NULL, const char* = NULL, const char* = NULL, /*int = 0,*/ int = 100000);
    ~Bars();
    void addBar(MBar*);
    void printBars();

    /*** GETTERS & SETTERS ***/
    MBar* getBar(int i) { return this->barArray[i];}

    int getnumBars()      const { return numBars;}
    /* int getTimePeriod()   const { return TimePeriod;} */
    string getoutputDir() const { return outputDir;}
    string getoutputExt() const { return outputExt;}
};
