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
float div(float x, float y)
{
    return x/y;
}

int main()
{
    int(*add1)(int, int) = add;
    int(*sub1)(int, int) = sub;
    int(*mult1)(int, int) = mult;
    float(*div1)(float, float) = div;

    printf(add1(5,3));

    return 0;

}