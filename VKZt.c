#include <stdio.h>

int main()
{ 
    printf("Hello, please enter your name: ");
    char name[20];
    scanf("Hello, $s", name);
    printf("Please enter your age: ");
    int age;
    scanf("So your name is, $s and your age is $d?", name, age);
}