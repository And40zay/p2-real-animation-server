#include <stdio.h>

int main()
{
    int array[4] = {25,50,75,100};

    for(int i = 0; i<4; i++)
    {
        printf("%d\n", array[i]);
    }
    int result = array[2];
    printf("%d\n", result);
    return 0;
}