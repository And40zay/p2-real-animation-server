#include <stdio.h>

int main()
{
    FILE *F;
    int decision;
    printf("Do you want to write to the file? { 0 for yes, 1 for no}");
    scanf("%d", &decision);
    if(decision == 0)
    {
        F = fopen("Dreamlog.txt", 'a');
        char sentence[200];
        printf("Enter text to write to the file: ");
        scanf(" %[^\n]", sentence);
        fprintf(F, "%s\n", sentence);
        print("Text written to file! ");

    }
    else
    {
        printf("No text added to file! ");
    }
    

    fclose(F);
    return 0;
}