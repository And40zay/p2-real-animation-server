#include <stdio.h>
#include <stdlib.h>

int main() {
    // Normal variables
    int myInt = 42;
    char myChar = 'A';
    double myDouble = 3.14;

    // Arrays
    char sentence[20];
    double classroom[5];

    // Print sizes of normal variables
    printf("Size of int: %zu bytes\n", sizeof(myInt));
    printf("Size of char: %zu bytes\n", sizeof(myChar));
    printf("Size of double: %zu bytes\n", sizeof(myDouble));
    printf("Size of sentence[20]: %zu bytes\n", sizeof(sentence));
    printf("Size of classroom[5]: %zu bytes\n", sizeof(classroom));

    // malloc: allocate 5 ints
    int *arr1 = malloc(5 * sizeof(int));
    if (arr1 == NULL) 
    {
        printf("malloc failed!\n");
        return 1;
    }
    printf("malloc: allocated %zu bytes at address %p\n", 5 * sizeof(int), (void*)arr1);

    // calloc: allocate 5 ints (zero-initialized)
    int *arr2 = calloc(5, sizeof(int));
    if (arr2 == NULL) 
    {
        printf("calloc failed!\n");
        free(arr1);
        return 1;
    }
    printf("calloc: allocated %zu bytes at address %p\n", 5 * sizeof(int), (void*)arr2);

    // realloc: grow arr1 to 10 ints
    int *arr3 = realloc(arr1, 10 * sizeof(int));
    if (arr3 == NULL) 
    {
        printf("realloc failed!\n");
        free(arr1);
        free(arr2);
        return 1;
    }
    printf("realloc: resized arr1 to %zu bytes at address %p\n", 10 * sizeof(int), (void*)arr3);

    // Clean up
    free(arr2);
    free(arr3);

    return 0;
}
