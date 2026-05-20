#include <stdio.h>
#include <time.h>

int main()
{
    TimeAus();
    return 0;
}

typedef void myTime()
{
    time_t theTime;
    time(&theTime);

    printf("Current time in Sydney %s\n", ctime(&theTime));
}TimeAUS;