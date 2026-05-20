#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main()
{
    int students;
    printf("How many students are in the class? ");
    scanf("%d", &students);

    int *size = malloc(students * sizeof(int));

    if(size == NULL)
    {
        printf("Error! ");
        return 1;
    }

    int i;
    int grade[students];

    for(int i = 0; i<size; i++)
    {
        printf("Enter grade for student: %d ", i);
        scanf("%d", &grade[i]);

    }
    printf("GRADE REPORT: \n");

    int average;
    int max = grade[0];
    int min = grade[0];
    for(int i = 0; i<size; i++)
    {)
        average += grade[i];
        if(grade[i]> max)
        {
            max = grade[i];
        }
        if(grade[i]< min)
        {
            min = grade[i];
        }
    }
    char new;
    printf("Highest Grade: %d \n", max);
    printf("Average grade: %d \n", average/students);
    printf("Lowest grade: %d \n", min);
    printf("Did any new Students join? {Y/N}");
    scanf(" %c", &new);

    if(new == 'y')
    {
        int num1;
        printf("\nHow many new students joined? ");
        scanf("%d", &num1);
        int totalStudents = 

    }



    free(size);
    return 0;
}