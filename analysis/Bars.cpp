
#include "Bars.h"

#include <string>
#include <typeinfo>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Bars::Bar::Bar(char* dateTime, float open, float close, float high, float low, float vol)
    : open(open), close(close), high(high), low(low), vol(vol)
{
    this->dateTime = (char*) malloc(this->maxDateChar);
    strcpy(this->dateTime, dateTime);
}


void Bars::Bar::Delete()
{
    free(this->dateTime);
    delete(this);
}


Bars::Bars(int barsPerDay, int maxBars, const char* inputFileDir, const char* startDate, const char* endDate)
{
    this->barsPerDay = barsPerDay;
    this->maxBars = maxBars;

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
                        tempArray[d++] = new Bar(dateTime, open, close, high, low, vol);
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
                        tempArray[d++] = new Bar(dateTime, open, close, high, low);
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



