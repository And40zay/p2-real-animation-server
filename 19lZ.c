#include <stdio.h>

int main()
{
    FILE *F;
    F = fopen("Dreamlog.txt", "r");
    if(F == NULL)
    {
        printf("Error opening file! ");
        return 1;
    }
    else
    {
        char language[1000];
        fgets(language, 1000, F);
        printf("%s", language);
        
    }
    


    fclose(F);
    return 0;
}