#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>


int main()
{
    DIR *dir;
    struct dirent *entry;
    char cwd[1024];
    char *home_path;

    if((getcwd(cwd, sizeof(cwd)) != NULL))
    {
        printf("Current working directory: %s", cwd);
    }
    else
    {
        printf("getcwd() Error!");
        return 1;
    }
    
    
    home_path = getenv("HOME");
    if(home_path == NULL)
    {
        printf("getenv(home) error!");
        return 1;
    }
    printf("Home path is in: %s\n", home_path);

    dir = opendir(home_path);

    if(dir == NULL)
    {
        printf("Error dir");
        return 1;
    }
    count;
    while((entry = readdir(dir) != NULL)
    {
        count++;
        if(entry->d_name)

    }





    return 0;

}
