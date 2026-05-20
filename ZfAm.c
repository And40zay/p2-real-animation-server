#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main()
{
    char *home_path;
    DIR *dir;
    struct dirent *entry;

    printf("=====LINUX EXPLORER=====");

    printf("1. First, let's see where this program is running \n");

    char cwd[1024];

    if(getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("CURRENT WORKING DIRECTORY: %s\n", cwd);

    }
    else
    {
        perror("getcwd() Error! ");
    }

    printf("\n2. Finding your home direcotry: \n");
    
    home_path = getenv("ROOT");

    if(home_path == NULL)
    {
        printf("Could not find HOME enviornment variable\n");
        printf("Trying alternate methods!");

        return 1;
    }
    printf("Home directory is: %s\n", home_path);

    printf("\n3. Contents of your home direcotry:\n");
    printf("======================================\n");

    dir = opendir(home_path);
    if(dir == NULL)
    {
        perror("Error! ");
        return 1;
    }
    int count = 0;
    printf("Hidden files starting with '.' are shown\n\n ");

    while((entry = readdir(dir))!= NULL)
    {
        count++;
        if(entry->d_name[0] == '.')
        {
            printf(".%-20s (hidden\n)", entry->d_name);
        }
        else
        {
            printf("%s\n", entry->d_name);
        }
    }
    printf("\n   Total: %d items\n", count);

    if(errno != 0)
    {
        perror("readdir error!");
    }
    closedir(dir);
}