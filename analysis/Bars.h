
#ifndef __BARS_H
#define __BARS_H

#include <string>
#include <typeinfo>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Bar.cpp"

using namespace std;

class Bars
{
    const int TimePeriod; // minutes
    const int maxBars;
    int numBars;
    Bar** barArray;

    /****** FILE-PARSING ********/
    const string outputDir = "../data/";
    const string outputExt = ".txt";

    int parseFile(const char*, const char*, const char*);

    public:

    /*** PUBLIC INTERFACE ***/
    Bars(const char* = "../input/dax-futures-hourly.txt", const char* = NULL, const char* = NULL, int = 0, int = 10000);
    void printBars();
    void Delete();

    /*** GETTERS & SETTERS ***/
    Bar* getBar(int i) { return this->barArray[i]; }

    int getnumBars() const { return numBars; }
    int getTimePeriod() const { return TimePeriod; }

    string getoutputDir() const { return outputDir; }
    string getoutputExt() const { return outputExt; }
};


#endif