
#include "Bars.h"

#include <string>
#include <typeinfo>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Bars::Bar::Bar(float open, float close, float high, float low, float vol, date_t* date_time, char* date_time_str)
    : Open(open), Close(close), High(high), Low(low), Vol(vol), Date_Time(date_time), date_time_str(date_time_str)
{
    this->Date_Time = (date_t*) malloc(sizeof(date_t));
    if (date_time != NULL)
    {
        *this->Date_Time = *date_time;
    }
    this->date_time_str = (char*) malloc(this->maxDateChar);
    if (date_time_str != NULL)
    {
        strcpy(this->date_time_str, date_time_str);
    }
}

void Bars::Bar::Delete()
{
    if (this->date_time_str != NULL) { free(this->date_time_str); }
    if (this->date_time_str != NULL) { free(this->Date_Time); }
    delete(this);
}

Bars::Bars(const char* inputFileDir, const char* startDate, const char* endDate, int timePeriod, int maxBars)
    : TimePeriod(timePeriod), maxBars(maxBars)
{
    int d = this->parseFile(inputFileDir, startDate, endDate);
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

int Bars::parseFile(const char* inputFileDir, const char* startDate, const char* endDate)
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
    
    #ifdef INVESTING
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
            dateTime[j++] = *(p++);
        }
        float close, open, high, low, vol;
        sscanf(line, "%s %f %f %f %f %f", 
            dateTime, &close, &open, &high, &low, &vol);
        // Set record to true when we hit the end date:
        if (endDate != NULL && strcmp(dateTime, endDate) == 0) { record = true; }
        // Make Bar, and insert in tempArray, iff record is on:
        if (numLine != 0 && record)
        {
            tempArray[d++] = new Bar(open, close, high, low, vol, NULL, dateTime);
        }
        if (startDate != NULL && strcmp(dateTime, startDate) == 0) { break; }
        numLine++;
    } 
    #endif

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

    #ifdef HOURLY
        sscanf(line, "%s %s %f %f %f %f ",
        date, time, &open, &high, &low, &close);
        snprintf(dateTime, 20, "%s %s", date, time);
    #endif

    #ifdef DAILY
        sscanf(line, "%s %f %f %f %f ",
        dateTime, &open, &high, &low, &close);
    #endif

        // Set record to true when we hit the end date:
        if (endDate != NULL && strcmp(dateTime, endDate) == 0) { record = true; }
        // Make Bar, and insert in tempArray, iff record is on
        if (numLine != 0 && record)
        {
            tempArray[d++] = new Bar(open, close, high, low, 0.0f, NULL, dateTime);
        }
        if (startDate != NULL && strcmp(dateTime, startDate) == 0) { break; }
        numLine++;
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
        if (barArray[i]->date_time_str != NULL)
        {
            fprintf(fpArray[0], "%s\n", barArray[i]->date_time_str);
        }
        fprintf(fpArray[1], "%f\n", barArray[i]->open());
        fprintf(fpArray[2], "%f\n", barArray[i]->close());
        fprintf(fpArray[3], "%f\n", barArray[i]->high());
        fprintf(fpArray[4], "%f\n", barArray[i]->low());
        fprintf(fpArray[5], "%f\n", barArray[i]->vol());
        fprintf(fpArray[6], "%s\n", asctime(barArray[i]->date_time()));
    }
    for (int i=0; i < params; i++) { fclose(fpArray[i]); }
}

