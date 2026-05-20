#include <stdio.h>

int main()
{
    printf("Hello, What is your name?");
    char myName[20];
    scanf("%c",&myName);
    printf("Hello, %s", myName);
    return 0;
}