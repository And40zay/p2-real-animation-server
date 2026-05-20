#include <stdio.h>
#include <stdlib.h>


int main()
{
    int myInt;
    char sentence[20];

    printf("%zu\n", sizeof(myInt));
    printf("%zu", sizeof(sentence));

    return 0;
}