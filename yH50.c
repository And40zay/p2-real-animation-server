#include <stdio.h>
#include <stdlib.h>

typedef struct{
    char name[50];
    int EID;
    char department[50];
    float salary;
    int YOS;
} Employee;






int main()
{
    printf("How many employees do you have? ");
    int numberOfEmployees;
    scanf("%d", numberOfEmployees);
    while(getchar() != '\n');

    Employee *numEmployee = malloc(numberOfEmployees * sizeof(Employee));

    if(numEmployee = NULL)
    {
        printf("Error!");
        return 1;
    }   

    for(int i = 0; i<numberOfEmployees; i++)
    {
        printf("Enter Employee Name: ");
        scanf("%s", Employee[i].name);
        while(getchar() != '\n');
        printf("En")
    }

}