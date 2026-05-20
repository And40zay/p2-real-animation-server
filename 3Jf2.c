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
    char a;

    printf("Enter 1 for 'add' and 2 for 'sub'");

    scanf("%c", a);

    if(a == "a")
    {
        ptr = add;
    }
    else if(a == "b")
    {
        ptr == sub;
    }
    else
    {
        
    }

    int result = ptr(10, 10);
    printf("%d\n", result);


}