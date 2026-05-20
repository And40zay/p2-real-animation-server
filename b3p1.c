#include <stdio.h>S

int main(int argc, char *argv[])
{
    printf("Number of args: %d\n", argc);

    for(int i = 0; i<argc; i++)
    {
        printf("Args: %s\n", argv[i]);

    }
    return 0;
}