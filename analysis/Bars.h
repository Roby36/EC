
#ifndef __BARS_H
#define __BARS_H

using namespace std;

#include <string>


class Bars
{
    /****** GENERAL BAR STRUCTURE ********/
    int barsPerDay;
    int maxBars;
    int numBars;

    class Bar
    {
        const int maxDateChar = 32;
        char* dateTime;
        float open, close, high, low, vol;

        public:
        Bar(char*, float, float, float, float, float);
        Bar(char line[]);

        /*** GETTERS & SETTERS ***/
        char* getdateTime() const { return dateTime; }
        float getopen() const { return open; }
        float getclose() const { return close; }
        float gethigh() const { return high; }
        float getlow() const { return low; }
        float getvol() const { return vol; }
    };

    Bar** barArray;

    /****** FILE-PARSING ********/
    const string outputDir = "../graphs/data/";
    const string outputExt = ".txt";

    int parseFile(int, const char*, const char*, const char*);

    public:

    /*** PUBLIC INTERFACE ***/
    Bars(int = 1, int = 10000, const char* = "../data/DAX Historical Data.csv", const char* = "02/24/2022", const char* = "02/24/2023");
    void printBars();


    /*** GETTERS & SETTERS ***/
    Bar* getBar(int i) { return this->barArray[i]; }

    int getBarsPerDay() const { return this->barsPerDay; }
    int getnumBars() const { return numBars; }

    string getoutputDir() const { return outputDir; }
    string getoutputExt() const { return outputExt; }

};


#endif