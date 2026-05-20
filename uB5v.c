#include <stdio.h>

int main()
{
    FILE *file;
    file = fopen("Hello.txt", "a");
    fprintf(file, "Hello, world!");

    

    fclose(file);
}