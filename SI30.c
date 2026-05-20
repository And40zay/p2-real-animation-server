#include <stdio.h>
#include <stdlib.h>

int main() {
    int i;

    // malloc: allocate space for 5 integers (uninitialized)
    int *arr1 = malloc(5 * sizeof(int));
    if (arr1 == NULL) 
    {
        printf("malloc failed!\n");
        return 1;
    }
    for (i = 0; i < 5; i++) 
    {
        arr1[i] = i + 1;  // manually initialize
    }
    printf("arr1 (malloc): ");
    for (i = 0; i < 5; i++) 
    {
        printf("%d ", arr1[i]);
    }
    printf("\n");

    // calloc: allocate space for 5 integers (initialized to 0)
    int *arr2 = calloc(5, sizeof(int));
    if (arr2 == NULL) 
    {
        printf("calloc failed!\n");
        free(arr1);
        return 1;
    }
    printf("arr2 (calloc): ");
    for (i = 0; i < 5; i++) 
    {
        printf("%d ", arr2[i]);  // prints 0s
    }
    printf("\n");

    // realloc: grow arr1 to hold 10 integers
    int *arr3 = realloc(arr1, 10 * sizeof(int));
    if (arr3 == NULL) 
    {
        printf("realloc failed!\n");
        free(arr1);
        free(arr2);
        return 1;
    }
    // initialize new elements
    for (i = 5; i < 10; i++) 
    {
        arr3[i] = i + 1;
    }
    printf("arr3 (realloc from arr1): ");
    for (i = 0; i < 10; i++) 
    {
        printf("%d ", arr3[i]);
    }
    printf("\n");

    // free memory
    free(arr2);
    free(arr3);

    return 0;
}