#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>


int main()
{
    DIR *dir;
    struct dirent *entry;
    char cwd[1024];
    struct stat file_info;

    dir = opendir("/");
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
        if(entry->d_name[0] == '.')
        {
            continue;
        }

        if(lstat(entry->d_name & file_info == -1))
        {
            printf("error!");
            return 1;
        }

    }
    return 0;
}