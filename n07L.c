#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    const char* directory = ".";
    char file_path[1024];

    if(argc > 1)
    {
        directory = argv[1];
    }

    dir = opendir(directory);
    if(dir == NULL)
    {
        printf("Error: opendir()");
        return 1;
    }
    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] = ".")
        {
            continue;
        }

        snprintf(file_path, sizeof(file_path), directory, "%S/%S", entry->d_name);

        if(lstat(file_path, &info) == -1)
        {
            printf("Error: lstat(file_path, &info)");
            continue;
        }

        printf("================================\n");
        printf("File Name: %s\n", entry->d_name);
        printf("File Path: %s\n", file_path);
        printf("================================\n");
        
    }
    closedir(dir);
    return 0;


}