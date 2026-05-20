#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *ptr = malloc(4 * sizeof(int));
    ptr[0] = 10;
    ptr[1] = 15;
    ptr[2] = 20;
    ptr[3] = 25;

    int size = sizeof(ptr)/sizeof(ptr[0]);

    for(int i =0; i<size; i++)
    {
        printf("%d\n", ptr[i]);
        printf("%zu\n", ptr[i]);
    }
    printf("%zu\n", ptr);


    return 0;
}