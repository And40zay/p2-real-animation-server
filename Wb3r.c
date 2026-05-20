#include <stdio.h>
#include <stdbool.h>


float add(float x, float y)
{
    return x+y;
}
float sub(float x, float y)
{
    return x-y;
}
float mul(float x, float y)
{
    return x*y;
}
float div(float x, float y)
{
    return x/y;
}

int main()
{

    float (*something[4])(int, int) = {add, sub, mul, div};
    printf("Enter your first number: ");
    float num1;
    scanf("%f", &num1);
    printf("Enter your second number: ");
    float num2;
    scanf("%f", &num2);
    printf("Enter your operator: ");
    char op;
.    float result;

    switch(op)
    {
        case '+':
            result = add(num1, num2);
            break;
        case '-':
            result = sub(num1, num2);
            break;
        case '*':
            result = mul(num1, num2);
            break;
        case '/':
            result = div(num1, num2);
            break;
        default:
            printf("Invalid operator!!!");
            return 1;

    }
    printf("%f", result);
    return 0;



}



