#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *ptr;

    ptr = calloc(4, sizeof(*ptr));

    *ptr = 2;

    ptr[0] = 11;
    ptr[1] = 12;
    ptr[2] = 100;
    ptr[3] = 10;

    int size = sizeof(ptr)/sizeof(ptr[0]);

    for(int i = 0; i<size; i++)
    {
        printf("%d", &ptr[i]);
        printf("%zu", &ptr[i]);
    }

    return 0;

    


}