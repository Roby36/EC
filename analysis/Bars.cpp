
#include "Bars.h"

#include <string>
#include <typeinfo>
#include <ctime>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Bars::Bar::Bar(float open, float close, float high, float low, float vol,
  char* dateTime)
    : Open(open), Close(close), High(high), Low(low), Vol(vol)
{
    // Generate localDateTime struct and dateTime string internally at time of cnstruction
    time_t currtime = time(0);
    this->locDateTime = (tm*) malloc(sizeof(tm));
    *this->locDateTime = *(localtime(&currtime));
        
    this->dateTime = (char*) malloc(this->maxDateChar);
    if (dateTime != NULL)
    {
        strcpy(this->dateTime, dateTime);
    }
    else
    {
        strcpy(this->dateTime, ctime(&currtime));
    }
}


void Bars::Bar::Delete()
{
    free(this->dateTime);
    free(this->locDateTime);
    delete(this);
}


Bars::Bars(int timePeriod, int barsPerDay, int maxBars, const char* inputFileDir, const char* startDate, const char* endDate)
    : TimePeriod(timePeriod), maxBars(maxBars)
{
    int d = this->parseFile(barsPerDay, inputFileDir, startDate, endDate);
    if (d == 0)
    {
        fprintf(stderr, "Error extracting data: please enter valid parameters\n");
        exit(1);
    }
    else
    {
        this->numBars = d;
    }
}


void Bars::Delete()
{
    for (int i = 0; i < this->numBars; i++)
    {
        this->barArray[i]->Delete();
    }
    delete this->barArray;
    delete(this);
}


void Bars::printBars()
{
    int params = 7;
    string arr[] = {"dateTime", "open", "close", "high", "low", "vol", "locDateTime"};
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
        fprintf(fpArray[0], "%s\n", barArray[i]->date());
        fprintf(fpArray[1], "%f\n", barArray[i]->open());
        fprintf(fpArray[2], "%f\n", barArray[i]->close());
        fprintf(fpArray[3], "%f\n", barArray[i]->high());
        fprintf(fpArray[4], "%f\n", barArray[i]->low());
        fprintf(fpArray[5], "%f\n", barArray[i]->vol());
        fprintf(fpArray[6], "%s\n", asctime(barArray[i]->localDateTime()));
    }
    for (int i=0; i < params; i++) { fclose(fpArray[i]); }
}


int Bars::parseFile(int barsPerDay, const char* inputFileDir, const char* startDate, const char* endDate)
{
    Bar *tempArray[this->maxBars];
    FILE *fp = fopen(inputFileDir, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening %s\n", inputFileDir);
        return 0;
    }
    int d = 0;
    char c;
    char line[1024];
    int i;
    int numLine = 0;
    bool record = (endDate == NULL);
        switch(barsPerDay)
        {
            case 1:
            {
                while (numLine < this->maxBars)
                {
                    i = 0;
                    while ((c = fgetc(fp)) != '\n')
                    {
                        if (c == 'M')
                        {
                            while ((c = fgetc(fp)) != '\n' && c != EOF) {}
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
                    if (c == EOF) { break; }
                    char dateTime[12];
                    int j = 0;
                    char* p = line;
                    while (isspace(*p)) { p++; }
                    while (!isspace(*p))
                    {
                        dateTime[j] = *p;
                        j++;
                        p++; 
                    }
                    float close, open, high, low, vol;
                    sscanf(line, "%s %f %f %f %f %f", 
                        dateTime, &close, &open, &high, &low, &vol);
                             
                    // Set record to true when we hit the end date:
                    if (endDate != NULL && strcmp(dateTime, endDate) == 0) { record = true; }
                    // Make Bar, and insert in tempArray, iff record is on:
                    if (numLine != 0 && record)
                    {
                        tempArray[d++] = new Bar(open, close, high, low, vol, dateTime);
                    }
                    if (startDate != NULL && strcmp(dateTime, startDate) == 0) { break; }
                    numLine++;
                }
                break; 
            }
            case 9:
            case 21:
            {
                while (numLine < this->maxBars)
                {
                    i = 0;
                    while ((c = fgetc(fp)) != '\n')
                    {
                        if (c == EOF) { break; }
                        else if (c == ',')
                        {
                            line[i] = '.';
                        }
                        else
                        {
                            line[i] = c;
                        }
                        i++;
                    }
                    if (c == EOF) { break; }
                    char date[10];
                    char time[9];
                    char dateTime[20];
                    float open, high, low, close;
                    sscanf(line, "%s %s %f %f %f %f ",
                    date, time, &open, &high, &low, &close);
                    snprintf(dateTime, 20, "%s %s", date, time);
                    // Set record to true when we hit the end date:
                    if (endDate != NULL && strcmp(dateTime, endDate) == 0) { record = true; }
                    // Make Bar, and insert in tempArray, iff record is on:
                    if (numLine != 0 && record)
                    {
                        tempArray[d++] = new Bar(open, close, high, low, 0.0f, dateTime);
                    }
                    if (startDate != NULL && strcmp(dateTime, startDate) == 0) { break; }
                    numLine++;
                }
                break;
            }
            default: return 0;
        }
    fclose(fp);
    // Construct barArray containing all Bars, and delete old tempArray:
    this->barArray = new Bar*[d];
    for (int i = 0; i < d; i++)
    {
        this->barArray[i] = tempArray[d - 1 - i]; 
    }
    // Return days recorded:
    return d;
}

