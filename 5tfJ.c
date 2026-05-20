#include <stdio.h>

int main()
{
    printf("Hello, What is your name?");
    char myName[20];
    scanf("%s", myName);
    printf("Hello, %s", myName);
    return 0;
}