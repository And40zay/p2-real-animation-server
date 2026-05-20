#include <stdio.h>

int sum(int a, int b) {
    return a + b;
}

int main(void) {
    int a = 3, b = 4;
    printf("%d + %d = %d\n", a, b, sum(a, b));
    return 0;
}
