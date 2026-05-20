#include <stdio.h>
#include <stdbool.h>


float add(float x, float y);
float multiply(float x, float y);
float divide (float x, float y);
float subtract(float x, float y);

float add(float x, float y)
{
    printf("%f", x+y);
    return x + y;
}

float multiply(float x, float y)
{
    printf("%f", x*y);
    return x*y;
}

float subtract(float x, float y)
{
    printf("%f", x-y);
    return x-y; 
}

float divide(float x, float y)
{
    printf("%f", x/y);
    return x/y;
}

int main()
{
    bool tick = false;
    
    while(tick == false)
    {
        printf("Enter operation, a for add, m for multiply, s for subtract and d for divide of you can exit by pressing x: ");
        char letter;
        scanf("%c", letter);
        if(letter == 'x')
        {
            break;
        }
        printf("Enter first number: ");
        float num1;
        scanf("%f", &num1);
        printf("Enter second number: ");
        float num2;
        scanf("%f", num2);

        if(letter == 'a')
        {
            float add(float num1, float num2);
        }
        else if(letter == 'm')
        {
            float multiply(float num1, float num2);
        }
        else if(letter == 's')
        {
            float subtract(float num1, float num2);
        }
        else if(letter == 'd')
        {
            float divide(float num1, float num2);
        }
        else
        {
            tick = true;
        }

    }

}