#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char file_path[1024];


    dir = opendir("/");
    if(dir == NULL)
    {
        printf("Error opendir()");
        return 1;
    }
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
        {
            continue;
        }

        snprintf(file_path, sizeof(file_path), "/s", entry->d_name);

        if(lstat(file_path, &info) == -1)
        {
            printf("Cannot find file /s", entry->d_name);
            continue;
        }
        printf("============================\n");
        printf("File name: \n", entry->d_name);
        printf("File path: \n", file_path);
        printf("============================")
    }

}
