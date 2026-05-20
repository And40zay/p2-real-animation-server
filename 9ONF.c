#include <stdio.h>

int main()
{
    FILE *fp;
    int decision;
    printf("Do you want to write to the file? { 0 for yes, 1 for no}");
    scanf("%d", &decision);
    if(decision == 0)
    {
        fp = fopen("Dreamlog.txt","a");
        char sentence[200];
        printf("Enter text to write to the file: ");
        scanf(" %[^\n]", sentence);
        fprintf(fp, "%s\n", sentence);
        printf("Text written to file! ");

    }
    else
    {
        printf("No text added to file! ");
    }
    

    fclose(fp);
    return 0;
}