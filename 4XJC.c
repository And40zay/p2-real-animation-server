#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

int main()
{
    DIR *dir = opendir(".");
    if(dir == NULL)
    {
        printf("error!");
        return 1;
    }
    struct dirent *entry = readdir(dir);

    if(entry == NULL)
    {
        printf("error!");
        return 1;
    }
    closedir(dir);
    return 0;

}