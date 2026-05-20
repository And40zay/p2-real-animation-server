#include <stdio.h>
#include <stdbool.h>

int main()
{
    bool authenticator = false;
    while(authenticator == false)
    {
        printf("Please enter your name: ");
        char name[20];
        scanf("%s", name);
        printf("Hello %s, what is your age? ", name);
        int age;
        scanf("%d", &age);
        printf("If your name is %s and your age is %d please type 0 to confirm details! ", name, age);
        int correct;
        scanf("%d", &correct);
        if(correct == 0)
        {
            authenticator = true;     
        }
        else
        {
            authenticator = false;
        }

    }
    return 0;
    

nt }