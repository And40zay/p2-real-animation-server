#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char title[49];
    char director[49];
    int year;
    float rating;

}Movie;



int main()
{
    printf("How many movies in your collection? ");
    int movieCollection;
    scanf("%d", &movieCollection);

    Movie *movies = malloc(movieCollection * sizeof(Movie));

    if(movies == NULL)
    {
        printf("Error! ");
        return 1;
    }
    

    for(int i = 0; i<movieCollection; i++)
    {
        printf("Movie: %d", i+1);
        printf("Enter title: ");
        scanf("%s", movies[i].title);
        printf("Enter director: ");
        scanf("%s", movies[i].director);
        printf("Enter year made: ");
        scanf("%d", &movies[i].year);
        printf("Enter rating {0-100} ");
        scanf("%f", &movies[i].rating);
    }

    for(int i = 0; i<movieCollection; i++)
    {
        printf("Movie: %d", i+1);
        printf("Title: %s", movies[i].title);
        printf("Director: %s", movies[i].director);
        printf("Year made: %d", movies[i].year);
        printf("Rating: %f", movies[i].rating);
    }
    char yes;
    printf("Are there more movies you would like to buy? {Y,N}");
    scanf(" %c", &yes);
       
    if(yes == 'Y'|| yes == 'y')
    {
        printf("How many movies would you like to buy? ");
        int movieExtra;
        scanf("%d", &movieExtra);
        int newTotal = movieExtra + movieCollection;

        Movie *temp = realloc(movies, newTotal * sizeof(Movie));

        if(temp == NULL)
        {
            printf("Error! ");
            return 1;
        }
        
        for(int i = movieCollection; i<newTotal; i++)
        {
            printf("Movie: %d", i+1);
            printf("Enter title: ");
            scanf("%s", movies[i].title);
            printf("Enter director: ");
            scanf("%s", movies[i].director);
            printf("Enter year made: ");
            scanf("%d", &movies[i].year);
            printf("Enter rating {0-100} ");
            scanf("%f", &movies[i].rating);
        }

        for(int i = 0; i<newTotal; i++)
        {
            printf("Movie: %d", i+1);
            printf("Title: %s", movies[i].title);
            printf("Director: %s", movies[i].director);
            printf("Year made: %d", movies[i].year);
            prinf("Rating: %f", movies[i].rating);
        }


    }

    return 0;



}
