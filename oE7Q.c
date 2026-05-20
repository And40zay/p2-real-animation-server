#include <stdio.h>

int main()
{
    printf("Enter the number of elements: ");
    int element;
    scanf("%d", &element);
    int hi[element];
    for(int i = 0; i<element; i++)
    {
        scanf("%d", hi[i]);
    }
    for(int i = 0; i<element; i++)
    {
        printf("%d", hi[i]);
    }
    return 0;

}