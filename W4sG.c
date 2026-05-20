#include <stdio.h>

int add(int x, int y)
{
    return x+y;
}
int sub(int x, int y)
{
    return x-y;
}
int mult(int x, int y)
{
    return x*y;
}
int div(int x, int y)
{
    return x/y;
}

int main()
{
    int(*ptr)(int, int);

    char something;
    printf("Enter 'a' for add, 's' for sub, 'm' for mult or 'd' for div")
    scanf("%c", &something);

    if(something == 'a')
    {
        ptr = add;
    }
    else if(something == 's')
    {
        ptr = sub;
    }
    else if(something == 'm')
    {
        ptr = mult;
    }
    else if(something == 'd')
    {
        ptr = div;
    }
    int result1 = ptr(20,10);
    printf("%d", result1);




    return 0;
}