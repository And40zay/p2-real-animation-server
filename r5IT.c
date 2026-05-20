#include <stdio.h>
#include <time.h>

int main()
{
  
    myFunction();
    return 0;
}
void myFunction()
{
    time_t theTime;
    time(&theTime);

    printf("Time in Sydney AUstralia: %s\n", ctime(&theTime));
}