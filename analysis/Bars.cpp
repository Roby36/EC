
#include "Bars.h"

#include <string>
#include <map>
#include <typeinfo>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace std;


Bars::Bar::Bar(char* dateTime, float open, float close, float high, float low, float vol)
    : dateTime(dateTime), open(open), close(close), high(high), low(low), vol(vol)
{}

Bars::Bar::Bar(char line[])
{
    dateTime = (char*) malloc(maxDateChar);

    sscanf(line, "%s %f %f %f %f %f", 
    dateTime, 
    &close, 
    &open, 
    &high, 
    &low, 
    &vol);
}

Bars::Bars(int timePeriod, int maxBars, const char* inputFileDir, const char* startDate, const char* endDate)
{
    this->settimePeriod(timePeriod);
    this->maxBars = maxBars;

    switch (timePeriod)
    {
    case 24:
        this->setnumBars(this->parseDaily(inputFileDir, startDate, endDate));
        break;
    
    default:
        fprintf(stderr, "Error: only daily case is currently defined\n");
        exit(1);
    }
}


void Bars::printBars()
{
    int params = 6;
    string arr[] = {"dateTime", "open", "close", "high", "low", "vol"};
    FILE* fpArray[params]; 
    int i = 0;
    for (string str : arr) 
    {
        //First ensure file clear:
        FILE* fp = fopen((this->outputDir + str + this->outputExt).c_str(), "w"); 
        if (fp != NULL) { fclose(fp); }
        // Append data to file
        fp = fopen((this->outputDir + str + this->outputExt).c_str(), "a"); 
        if (fp == NULL)
        {
            fprintf(stderr, "Error opening some bar data directory for appending.\n");
            return;
        }
        fpArray[i] = fp; 
        i++;
    }
    for (int i = 0; i<numBars; i++)
    {
        fprintf(fpArray[0], "%s\n", barArray[i]->getdateTime());
        fprintf(fpArray[1], "%f\n", barArray[i]->getopen());
        fprintf(fpArray[2], "%f\n", barArray[i]->getclose());
        fprintf(fpArray[3], "%f\n", barArray[i]->gethigh());
        fprintf(fpArray[4], "%f\n", barArray[i]->getlow());
        fprintf(fpArray[5], "%f\n", barArray[i]->getvol());
    }
    for (int i=0; i < params; i++) { fclose(fpArray[i]); }
}


int Bars::parseDaily(const char* inputFileDir, const char* startDate, const char* endDate)
{
    Bar *tempArray[this->maxBars];

    FILE *fp = fopen(inputFileDir, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening %s\n", inputFileDir);
        return 0;
    }

    char c;
    char line[1024];
    int i;
    int numLine = 0;
    int d = 0;
    bool record = false;

    while (numLine < this->maxBars)
    {
        i = 0;
        while ((c = fgetc(fp)) != '\n')
        {
            if (c == 'M')
            {
                while ((c = fgetc(fp)) != '\n') {}
                break;
            }
            else if (c == ',')
            {
                continue;
            }
            else if (c == '"')
            {
                line[i] = ' ';
            }
            else
            {
                line[i] = c;
            }
            i++;
        }

        char date[12];
        int j = 0;
        char* p = line;
        while (isspace(*p)) { p++; }
        while (!isspace(*p))
        {
            date[j] = *p;
            j++;
            p++; 
        }

        // Set record to true when we hit the end date:
        if (strcmp(date, endDate) == 0) { record = true; }

        // Make Bar, and insert in tempArray, iff record is on:
        if (numLine != 0 && record)
        {
            Bar *bar = new Bar(line);
            tempArray[d] = bar;
            d++;
        }

        // Set record to false when we hit the start date:
        if (strcmp(date, startDate) == 0) { record = false; }

        numLine++;
    }

    fclose(fp);

    // Construct barArray containing all Bars:

    this->barArray = new Bar*[d];

    for (int i = 0; i < d; i++)
    {
        this->barArray[i] = tempArray[d - 1 - i];
    }

    // Return days recorded:
    return d;
}


