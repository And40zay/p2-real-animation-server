#include <stdio.h>

int Something()
{
    printf("Hello world!");
    return 0;
}

/* Add a standard main() so the program can be executed directly.
   We keep the existing Something() function and call it from main(). */
int main(void)
{
    return Something();
}