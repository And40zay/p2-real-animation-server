#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    char title[50];
    char director[50];
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
        printf("Movie: %d\n", i+1);
        printf("Enter title: ");
        scanf("%49s", movies[i].title);
        printf("Enter director: ");
        scanf("%49s\n", movies[i].director);
        printf("Enter year made: ");
        scanf("%d\n", &movies[i].year);
        printf("Enter rating {0-100} ");
        scanf("%f\n", &movies[i].rating);
    }

    for(int i = 0; i<movieCollection; i++)
    {
        printf("Movie: %d\n", i+1);
        printf("Title: %s\n", movies[i].title);
        printf("Director: %s\n", movies[i].director);
        printf("Year made: %d\n", movies[i].year);
        printf("Rating: %f\n", movies[i].rating);
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
        movies = temp;
        
        for(int i = movieCollection; i<newTotal; i++)
        {
            printf("Movie: %d", i+1);
            printf("Enter title: ");
            scanf("%49s", movies[i].title);
            printf("Enter director: ");
            scanf("%49s\n", movies[i].director);
            printf("Enter year made: ");
            scanf("%d", &movies[i].year);
            printf("Enter rating {0-100} ");
            scanf("%f", &movies[i].rating);
        }

        for(int i = 0; i<newTotal; i++)
        {
            printf("Movie: %d", i+1);
            printf("Title: %s\n", movies[i].title);
            printf("Director: %s\n", movies[i].director);
            printf("Year made: %d\n", movies[i].year);
            printf("Rating: %f\n", movies[i].rating);
        }


    }
    free(movies);
    movies = NULL;

    return 0;



}
