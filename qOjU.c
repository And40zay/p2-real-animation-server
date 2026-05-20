#include <stdio.h>

int main()
{
    printf("Enter the number of elements: ");
    int element[2];
    scanf("%d", &element);
    int array[&element];
    for(int i = 0; i<element; i++)
    {
        scanf("%d", array[i]);
    }
    for(int i = 0; i<element; i++)
    {
        printf("%d", array[i]);
    }
    return 0;

}