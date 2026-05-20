#include <stdio.h>
#include <stdlib.h>

typedef struct{
    char title[50];
    char author[50];
    int year;
}Book;

int main()
{
    int numberOfBooks;
    printf("How many books do you have in the library intitally? ");
    scanf("%d", &numberOfBooks);

    Book *library = malloc(numberOfBooks * sizeof(Book));

    if(library == NULL)
    {
        printf("Error! ");
        return 1;
    }


    for(int i = 0; i<numberOfBooks; i++)
    {
        printf("Book: %d", i+1);

        printf("Enter title: ");
        scanf("%49s", library[i].title);
        printf("Enter Author: ");
        scanf("%49s", library[i].author);
        printf("Enter year made: ");
        scanf("%d", library[i].year);
    }

    for(int i = 0; i<numberOfBooks; i++)
    {
       printf("Book: %d", i+1);
       printf("Title: %s", library[i].title);
       printf("Author: %s", library[i].author);
       printf("Year created: %d", library[i].year);
    }
    char extra;
    printf("Did the library buy new books? {Y,N}");
    scanf(" %c", &extra);

    if(extra == 'Y'|| extra == 'y')
    {
        prinf("How many new books did the Library buy? ");
        int extraBooks;
        scanf("%d", &extraBooks);
        int newMax = 0;
        newMax = numberOfBooks + extraBooks;
        int *sum = realloc(library, newMax * sizeof(Book));

        if(sum == NULL)
        {
            printf("Error!");
            return 1;
        }

        for(int i = numberOfBooks; i<newMax; i++)
        {
            
            printf("Book: %d", i+1);
            printf("Enter title: ");
            scanf("%49s", library[i].title);
            printf("Enter Author: ");
            scanf("%49s", library[i].author);
            printf("Enter year made: ");
            scanf("%d", library[i].year);
        }

        for(int i = 0; i<newMax; i++)
        {
            printf("Book: %d", i+1);
            printf("Title: %s", library[i].title);
            printf("Author: %s", library[i].author);
            printf("Year created: %d", library[i].year);
        }

        free(library);
        free(sum);


    }





    return 0;
}