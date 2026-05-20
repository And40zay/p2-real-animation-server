#include <stdio.h>

long factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main(void) {
    for (int i = 0; i <= 10; ++i) {
        printf("%d! = %ld\n", i, factorial(i));
    }
    return 0;
}
