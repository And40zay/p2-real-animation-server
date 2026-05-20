#include <stdio.h>
#include <stdlib.h>


int main()
{
    int myInt;
    char sentence[20];
    char myChar;
    double myDouble;
    double classroom[5];
   

    printf("%zu\n", sizeof(myInt));
    printf("%zu\n", sizeof(sentence));
    printf("%zu\n", sizeof(myChar));
    printf("%zu\n", sizeof(&myDouble));
    printf("%zu\n", sizeof(classroom));
    int *ptr1 = malloc(sizeof(int));

    return 0;
}