
#include <ctime>
#include <stdio.h>

using namespace std;

int main()
{
    time_t currtime = time(0);

    char* currTimeString = ctime(&currtime);

    fprintf(stdout, "%s\n", currTimeString);
    
    return 0;
}