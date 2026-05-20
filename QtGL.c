#include <stdio.h>
#include <time.h>

int main()
{
    for(int i = 0; i<10; i++)
    {
        myFunction();
    }
    return 0;
}
void myFunction()
{
    time_t theTime;
    time(&theTime);

    printf("Time in Sydney AUstralia: %s\n", ctime(&theTime));
}