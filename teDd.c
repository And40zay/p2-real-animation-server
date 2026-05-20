#include <stdio.h>

int sum(int arr[], int n) {
    int s = 0;
    for (int i = 0; i < n; ++i) s += arr[i];
    return s;
}

int main(void) {
    int arr[] = {1, 2, 3, 4, 5};
    int n = sizeof(arr) / sizeof(arr[0]);
    printf("Sum of array is: %d\n", sum(arr, n));
    return 0;
}
