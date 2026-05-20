#include <stdio.h>
#include <time.h>

int main()
{
    time_t timeNow;
    time(&timeNow);

    printf("The current time is Sydney is: %d ", ctime(timeNow));
    return 0;
}