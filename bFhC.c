#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    char cwd[1024];

    dir = opendir("\\");
    if(dir == NULL)
    {
        printf("Error: opendir('')");
        return 1;
    }

    if((getcwd(cwd, sizeof(cwd))) != NULL)
    {
        printf("CWD: %s", cwd);
    }

    while((entry = readdir(dir)) != NULL)
    {
        printf("%s/n", entry->d_name);
    }
    return 0;
}