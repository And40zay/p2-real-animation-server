#include <stdio.h>
#include <time.h>

int main()
{
    myFunction();
    myFunction();
    return 0;
}

void myFunction()
{
    time_t theTime;
    time(&theTime);

    printf("Current time in Sydney %s\n", ctime(&theTime));
}