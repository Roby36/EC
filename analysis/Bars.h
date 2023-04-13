
#ifndef __BARS_H
#define __BARS_H

#include <string>
#include <ctime>

using namespace std;

class Bars
{
    /****** GENERAL BAR STRUCTURE ********/
    const int TimePeriod; // minutes
    const int maxBars;
    int numBars;

    class Bar
    {
        const int maxDateChar = 20;
        char* dateTime;
        struct tm* locDateTime;
        const float Open, Close, High, Low, Vol;

        public:

        Bar(float, float, float, float, float = 0.0f,
         char* = NULL);
        void Delete();

        /*** GETTERS & SETTERS ***/
        struct tm* localDateTime() const { return locDateTime; }
        char* date() const { return dateTime; }
        float open() const { return Open; }
        float close() const { return Close; }
        float high() const { return High; }
        float low() const { return Low; }
        float vol() const { return Vol; }
    };

    Bar** barArray;

    /****** FILE-PARSING ********/
    const string outputDir = "../data/";
    const string outputExt = ".txt";

    int parseFile(int, const char*, const char*, const char*);

    public:

    /*** PUBLIC INTERFACE ***/
    Bars(int = 0, int = 9, int = 10000, const char* = "../input/dax-futures-hourly.txt", const char* = NULL, const char* = NULL);
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