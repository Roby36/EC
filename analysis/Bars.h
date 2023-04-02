
#ifndef __BARS_H
#define __BARS_H

#include <string>

using namespace std;

class Bars
{
    /****** GENERAL BAR STRUCTURE ********/
    int barsPerDay;
    int maxBars;
    int numBars;

    class Bar
    {
        const int maxDateChar = 20;
        char* dateTime;
        float open, close, high, low, vol;

        public:

        Bar(char*, float, float, float, float, float = 0.0f);
        void Delete();

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
    const string outputDir = "../data/";
    const string outputExt = ".txt";

    int parseFile(int, const char*, const char*, const char*);

    public:

    /*** PUBLIC INTERFACE ***/
    Bars(int = 9, int = 10000, const char* = "../input/dax hourly.txt", const char* = NULL, const char* = NULL);
    void printBars();
    void Delete();


    /*** GETTERS & SETTERS ***/
    Bar* getBar(int i) { return this->barArray[i]; }

    int getBarsPerDay() const { return this->barsPerDay; }
    int getnumBars() const { return numBars; }

    string getoutputDir() const { return outputDir; }
    string getoutputExt() const { return outputExt; }
};


#endif