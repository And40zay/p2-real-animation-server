#include <stdio.h>
#include <time.h>

int main()
{
    time_t myTime;
    time(&myTime);

    printf("Current time in Sydney %s\n", ctime(&myTime));
    return 0;
}