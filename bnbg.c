#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char full_path[1024];
    

    dir = opendir("/");
    if(dir == NULL)
    {
        printf("Error: opendir('['])");
        return 1;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
        {
            continue;
        }

        snprintf(full_path, sizeof(full_path),"/%s" ,entry->d_name);

        if(lstat(full_path, &info) == -1 )
        {
            printf("Cannot access: /%s", entry->d_name);
            continue;
        }

        printf("=================================================\n");
        printf("File Name: %s\n", entry->d_name);
        printf("Full Path: %s\n", full_path);
        printf("=================================================");


    }
    return 0;
}
