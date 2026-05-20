#include <stdio.h>

int main()
{
    printf("Enter your name:");
    char name[20];
    scanf("%s", name);
    printf("Hello %s, enter your age: ", name);
    int age;
    scanf("%d", age);
    printf("Hello %s, you are %d years old", name, &age);
    return 0;
 