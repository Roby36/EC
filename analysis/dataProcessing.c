
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "day.h"
#include "dataProcessing.h"


const int MAXLINES = 2800;


/************************************** INPUT & OUTPUT PROCESSING **********************************************/

void resetBuffer(char *buffer)
{
    for (int i = strlen(buffer); i >= 0; i--)
    {
        buffer[i] = '\0';
    }
}


int parseFile(const char *fileDirectory, day_t *dayArray[], const char* startDate, const char* endDate)
{

    day_t *tempArray[MAXDAYS];

    FILE *fp = fopen(fileDirectory, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening %s\n", fileDirectory);
        return 0;
    }

    char c;
    char line[1024];
    int i;
    int numLine = 0;
    int d = 0;
    bool record = false;

    while (numLine < MAXLINES)
    {

        i = 0;

        while ((c = fgetc(fp)) != '\n')
        {
            if (c == 'M')
            {
                while ((c = fgetc(fp)) != '\n')
                {
                }
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
        while (!isspace(*p)){
            date[j] = *p;
            j++;
            p++; 
        }

        // Set record to true when we hit the end date:
        if (strcmp(date, endDate) == 0) { record = true; }

        // Make day struct iff record is on:
        if (numLine != 0 && record)
        {
            day_t *day = malloc(sizeof(day_t));
            // Initialize standard parameters
            sscanf(line, "%s %f %f %f %f %f", (char *)&day->date, &day->close, &day->open, &day->high, &day->low, &day->vol);
            // Initialize analysis parameters to false & RSI to 0:
            day->RSI = (float) 0;
            day->disagreement = false;
            day->consDisagreement = false;
            day->bearEngulf = false;
            day->bullEngulf = false;
            day->morningStar = false;
            day->eveningStar = false;
            day->hammer = false;
            day->llDoji = false;
            day->dfDoji = false;
            day->gsDoji = false;
            tempArray[d] = day;
            d++;
        }

        // Set record to false when we hit the start date:
        if (strcmp(date, startDate) == 0) { record = false; }

        resetBuffer(line);
        numLine++;
    }

    fclose(fp);

    // Construct array containing all days:
    for (int i = 0; i < d; i++)
    {
        dayArray[i] = tempArray[d - 1 - i];
    }

    // Return days recorded:
    return d;
}


void printDay(day_t *day)
{
    printf("%s %f %f %f %f %f   %f\n", (char *)day->date, day->close, day->open, day->high, day->low, day->vol, day->RSI);
}


void printDays(day_t *dayArray[], const int daysRecorded)
{
    for (int i = 0; i < daysRecorded; i++)
    {
        if (i == 0)
        {
            printf("Date         Close         Open        High         Low          Volume        RSI\n");
        }
        day_t *day = dayArray[i];
        printDay(day);
    }
}




/************************************** TEXT-FILES FOR GRAPHS **********************************************/

void printDates(day_t *dayArray[], const int daysRecorded, const char* fileDirectory){

    FILE* fp = fopen(fileDirectory, "a");
    if (fp == NULL){
        fprintf(stderr, "Error opening %s for appending.\n", fileDirectory);
        return;
    }

    for (int i = 0; i < daysRecorded; i++)
    {
        day_t *day = dayArray[i];
        fprintf(fp, "%s\n", day->date);
    }

    fclose(fp);
}


void printCloses(day_t *dayArray[], const int daysRecorded, const char* fileDirectory){

    FILE* fp = fopen(fileDirectory, "a");
    if (fp == NULL){
        fprintf(stderr, "Error opening %s for appending.\n", fileDirectory);
        return;
    }

    for (int i = 0; i < daysRecorded; i++)
    {
        day_t *day = dayArray[i];
        fprintf(fp, "%f\n", day->close);
    }

    fclose(fp);
}


void printVolumes(day_t *dayArray[], const int daysRecorded, const char* fileDirectory){

    FILE* fp = fopen(fileDirectory, "a");
    if (fp == NULL){
        fprintf(stderr, "Error opening %s for appending.\n", fileDirectory);
        return;
    }

    for (int i = 0; i < daysRecorded; i++)
    {
        day_t *day = dayArray[i];
        fprintf(fp, "%f\n", day->vol);
    }

    fclose(fp);
}


void printRSI(day_t *dayArray[], const int daysRecorded, const char* fileDirectory){

    FILE* fp = fopen(fileDirectory, "a");
    if (fp == NULL){
        fprintf(stderr, "Error opening %s for appending.\n", fileDirectory);
        return;
    }

    for (int i = 0; i < daysRecorded; i++)
    {
        day_t *day = dayArray[i];
        fprintf(fp, "%f\n", day->RSI);
    }

    fclose(fp);
}

