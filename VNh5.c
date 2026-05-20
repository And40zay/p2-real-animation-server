#include <stdio.h>
#include <stdbool.h>

int main()
{
    FILE *f;
    
    fopens("Diary.txt", 'a');
    char sentence[20];
    printf("Enter a sentence: ");
    scanf("%s\n", sentence);
    fprintf("Hello.txt", 'a');

    fclose(f);




}