
#include "Bars.h"

Bars::Bars(const char* inputFileDir, const char* startDate, const char* endDate)
{
    if (inputFileDir == NULL) return;
    this->numBars = this->parseFile(inputFileDir, startDate, endDate);
    if (this->numBars == 0) {
        fprintf(stderr, "Error extracting data: please enter valid parameters\n");
        exit(1);
    }
}

void Bars::addBar(MBar* bar) {
    if (numBars < MAXBARS) this->barArray[numBars++] = bar;
    else fprintf(stderr, "Maximum number of bars reached\n");
}

Bars::~Bars()
{
    for (int i = 0; i < this->numBars; i++) {
        delete(this->barArray[i]);
    }
}

void Bars::clear_file(const char* filename)
{
    FILE* fp = fopen(filename, "w"); 
    if (fp != NULL) fclose(fp);
}

void Bars::printBars(const std::string outputDir, const std::string name, const std::string outputExt)
{    
    FILE* fpArray[NUM_DATA]; 
    int i = 0;
    // Open files for appending data
    for (std::string str : DataStr) {
        char * output_path = (char *) malloc (OUTDIRCHAR);
        strncpy(output_path, (outputDir + name + str + outputExt).c_str(), OUTDIRCHAR);
        clear_file(output_path);
        FILE * fp = fopen(output_path, "a");
        if (fp == NULL)
            fprintf(stderr, "Error opening some bar data directory %s for appending.\n", output_path);
        fpArray[i++] = fp; 
        free(output_path);
    }
    // Append data to the files
    for (int i = 0; i < numBars; i++) {
        if (barArray[i]->date_time_str != NULL && fpArray[DATE_TIME] != NULL)
            fprintf(fpArray[DATE_TIME], "%s\n", barArray[i]->date_time_str);
        if (fpArray[OPEN]  != NULL) fprintf(fpArray[OPEN],  "%f\n", barArray[i]->open());
        if (fpArray[CLOSE] != NULL) fprintf(fpArray[CLOSE], "%f\n", barArray[i]->close());
        if (fpArray[HIGH]  != NULL) fprintf(fpArray[HIGH],  "%f\n", barArray[i]->high());
        if (fpArray[LOW]   != NULL) fprintf(fpArray[LOW],   "%f\n", barArray[i]->low());
        if (fpArray[VOL]   != NULL) fprintf(fpArray[VOL],   "%f\n", barArray[i]->vol());
        if (fpArray[LOC_DATE_TIME] != NULL) fprintf(fpArray[LOC_DATE_TIME], "%s\n", asctime(barArray[i]->date_time()));
    }
    // Close files after appending data
    for (int i = 0; i < NUM_DATA; i++) { 
        if (fpArray[i] != NULL) fclose(fpArray[i]);
    }
}

int Bars::parseFile(const char* inputFileDir, const char* startDate, const char* endDate)
{
    MBar *tempArray[MAXBARS];
    FILE *fp = fopen(inputFileDir, "r");
    if (fp == NULL) {
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
    while (numLine < this->maxBars) {
        i = 0;
        while ((c = fgetc(fp)) != '\n') {
            if (c == 'M') {
                while ((c = fgetc(fp)) != '\n' && c != EOF) 
                    {}
                break;
            } 
            else if (c == ',') continue;
            else if (c == '"') line[i] = ' ';
            else line[i] = c;
            i++;
        }
        if (c == EOF) break; 
        char dateTime[12];
        int j = 0;
        char* p = line;
        while (isspace(*p)) { 
            p++;
        }
        while (!isspace(*p)) {
            dateTime[j++] = *(p++);
        }
        double close = 0.0, open = 0.0, high = 0.0, low = 0.0, vol = 0.0;
        sscanf(line, "%s %f %f %f %f %f", dateTime, &close, &open, &high, &low, &vol);
        // Set record to true when we hit the end date:
        if (endDate != NULL && strcmp(dateTime, endDate) == 0) 
            record = true;
        // Make Bar, and insert in tempArray, iff record is on:
        if (numLine != 0 && record)
            tempArray[d++] = new Bar(open, close, high, low, vol, NULL, dateTime);
        if (startDate != NULL && strcmp(dateTime, startDate) == 0) break;
        numLine++;
    } 
    #endif
    while (numLine < MAXBARS) {
        i = 0;
        while ((c = fgetc(fp)) != '\n') {
            if (c == EOF) break;
            else if (c == ',') line[i] = '.';
            else line[i] = c;
            i++;
        }
        if (c == EOF) break;
        char date[10];
        char time[9];
        char dateTime[20];
        double open = 0.0, high = 0.0, low = 0.0, close = 0.0;
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
        if (endDate != NULL && strcmp(dateTime, endDate) == 0) record = true;
        // Make Bar, and insert in tempArray, iff record is on
        if (numLine != 0 && record)
            tempArray[d++] = new MBar(open, close, high, low, 0.0, NULL, dateTime);
        if (startDate != NULL && strcmp(dateTime, startDate) == 0) break;
        numLine++;
    }
    fclose(fp);
    // Construct barArray containing all Bars, and delete old tempArray:
    for (int i = 0; i < d; i++) {
        this->barArray[i] = tempArray[d - 1 - i]; 
    }
    // Return days recorded:
    return d;
}
