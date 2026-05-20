#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>

int main()
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(".");
    if(dir == 1)
    {
        perror("Error!");
        return 1;
    }

    printf("Contents of directory: \n");

    while(entry = readdir(dir) != NULL)
    {
        printf("%s\n", entry->d_name); 
    }

    closedir(dir);
    return 0;
}