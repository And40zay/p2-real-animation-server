#include <stdio.h>
#include <stdlib.h>


int main()
{
    int myInt;
    char sentence[20];
    char myChar;
    double myDouble;

    printf("%zu\n", sizeof(myInt));
    printf("%zu\n", sizeof(sentence));
    printf("%zu\n", sizeof(myChar));
    printf("%zu\n", sizeof(&myDouble));

    return 0;
}