
#ifndef __BARS_H
#define __BARS_H

#include <string>
#include <ctime>

typedef struct tm date_t;

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