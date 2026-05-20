#include <stdio.h>

// Exercise2.c — compute factorial (iterative) of a number read from input
// Uses scanf to get a non-negative integer from the user.

long factorial(int n) {
    long res = 1;
    for (int i = 2; i <= n; ++i) res *= i;
    return res;
}

int main(void) {
    int n;
    printf("Enter a non-negative integer: ");
    if (scanf("%d", &n) != 1) {
        fprintf(stderr, "Invalid input\n");
        return 1;
    }
    if (n < 0) {
        fprintf(stderr, "Please enter a non-negative integer\n");
        return 1;
    }
    printf("%d! = %ld\n", n, factorial(n));
    return 0;
}
