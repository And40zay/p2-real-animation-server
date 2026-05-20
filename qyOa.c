#include <stdio.h>
#include <time.h>

int main()
{
    time_t timeNow;
    time(&timeNow);

    printf("The current time is Sydney is: %s\n", ctime(&timeNow));
    return 0;
}