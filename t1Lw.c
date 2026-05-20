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

int do_math(int(*ptrr)(int,int),int x, int y)
{
    int result2 = ptrr(x,y);
    printf("%d", result2);

}

int main()
{
    int(*ptr)(int, int);

    char something;
    printf("Enter 'a' for add, 's' for sub, 'm' for mult or 'd' for div");
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

    do_math(add(10,20));




    return 0;
}