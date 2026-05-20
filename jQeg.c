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

int main()
{
    int(*ptr)(int, int);
    int a;
    int b;

    printf("Enter 1 for 'add' and 2 for 'sub'");

    scanf("%d\n", a);

    if(a == 1)
    {
        ptr = add;
    }
    else if(a == 2)
    {
        ptr == sub;
    }
    else
    {
        
    }

    int result = ptr(10, 10);
    printf("%d\n", result);


}