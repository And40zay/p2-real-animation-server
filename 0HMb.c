#include <stdio.h>

// Exercise1.c — sum elements of an array and print them
// IntelliCode is disabled in this workspace for C practice.

int main(void) {
    int myArray[] = {1, 2, 3, 4, 5};
    int n = sizeof(myArray) / sizeof(myArray[0]);
    int sum = 0;

    printf("Array elements:\n");
    for (int i = 0; i < n; ++i) {
        printf("%d ", myArray[i]);
        sum += myArray[i];
    }
    printf("\nSum = %d\n", sum);
    return 0;
}
