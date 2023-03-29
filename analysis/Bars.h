
#ifndef __BARS_H
#define __BARS_H

using namespace std;

#include <string>


class Bars
{
    /****** GENERAL BAR STRUCTURE ********/
    int timePeriod;
    int maxBars;
    int numBars;

    class Bar
    {
        const int maxDateChar = 20;
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


    /****** FILE-PARSING ********/
    const string outputDir = "../graphs/data/";
    const string outputExt = ".txt";
    int parseDaily(const char*, const char*, const char*);


    public:

    Bars(int = 24, int = 2800, const char* = "../data/DAX Historical Data.csv", const char* = "02/24/2022", const char* = "02/24/2023");
    Bar** barArray; 
    void printBars();


    /*** GETTERS & SETTERS ***/
    int gettimePeriod() const { return timePeriod; }
    void settimePeriod(int timePeriod) { this->timePeriod = timePeriod; }
    int getnumBars() const { return numBars; }
    void setnumBars(int numBars) { this->numBars = numBars; }
    string getoutputDir() const { return outputDir; }
    string getoutputExt() const { return outputExt; }

};


#endif