
#ifndef __DATAPROCESSING_H
#define __DATAPROCESSING_H

#define MAXDAYS 10000

#include "day.h"

/************************************** INPUT & OUTPUT PROCESSING **********************************************/

void resetBuffer(char *buffer);

int parseFile(const char *fileDirectory, day_t *dayArray[], const char* startDate, const char* endDate);

void printDay(day_t *day);

void printDays(day_t *dayArray[], const int daysRecorded);


/************************************** TEXT-FILES FOR GRAPHS **********************************************/

FILE* openDirectory(const char* fileDirectory);

void printDates(day_t *dayArray[], const int daysRecorded, const char* fileDirectory);

void printFloatAttributes(day_t *dayArray[], const int daysRecorded, const char* attribute, const char* fileDirectory);

void printBoolAttributes(day_t *dayArray[], const int daysRecorded, const char* attribute, const char* fileDirectory);



#endif // __DATAPROCESSING_H