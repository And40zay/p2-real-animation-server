#include <stdio.h>

int main()
{
    printf("Enter the number of elements: ");
    int element;
    scanf("%d", &element);
    int hi[element];
    for(int i = 0; i<element; i++)
    {
        scanf("%d", &hi[i]);
    }
    int j;
    for(int j = 0; j<element; j++)
    {
        printf("%d\n", hi[j]);
    }
    return 0;

}