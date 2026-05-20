#include <stdio.h>
#include <stdbool.h>

int main()
{   bool isAnswerGiven = false;
    while(isAnswerGiven == false)
    {
        printf("Enter your name: ");
        char name[20];
        scanf("%s", name);
        printf("Hello, %s please enter your age: ", name);
        int age;
        scanf("%d", age);
        
        printf("Type \"yes\" if your name is %s and your age is %d", name, age);
        char calling[3];
        scanf("%s", calling);
        if(calling == "yes")
        {
            isAnswerGiven = true;
        }
        else
        {
            continue;
        }
    }
}