#include <stdio.h>

int main()
{
    int array[4] = {25,50,75,100};

    printf("%d\n", *(array + 1 ));

    int *ptr = array;
    ptr+=2;

    printf("%d\n", *ptr);
}