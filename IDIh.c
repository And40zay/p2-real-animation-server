#include <stdio.h>
#include <stdbool.h>

int main()
{
    FILE *f;
    f = fopens("Hello.txt", 'a')
    char sentence[20];
    printf("Enter a sentence: ");
    scanf("%s\n", sentence);
    fprintf(sentence);

    fclose(f);




}