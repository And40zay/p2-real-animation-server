#include <stdio.h>
#include <stdbool.h>

float add(float x, float y);
float sub(float x, float y);
float mul(float x, float y);
float div(float x, float y);


int main()
{
    printf("Enter your first number: ");
    float num1;
    scanf("%f\n", &num1);
    printf("Enter your second number: ");
    float num2;
    scanf("%f\n", &num2);
    char op;
    printf("Enter your operator {+,-,/,*}");
    scanf(" %c", &op);
    float result;
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
        deafault:
            printf("Invalid operator");
            return 1;


    }
    printf("%f\n", &result);
    return 0;

    


}

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
    if(y != 0)
    {
        return x/y;
    }
    
}


