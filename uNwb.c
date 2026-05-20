#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[])
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char file_path[1024];
    const char* directory = ".";

    if(argc > 1)
    {
        directory = argv[1];
    }

    dir = opendir(directory);
    if(dir == NULL)
    {
        printf("Error: opendir(directory)");
        return 1;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
        {
            continue;
        }

        snprintf(file_path, sizeof(file_path), directory, "%s/%s", entry->d_name);

        if((lstat(file_path, &info) == -1))
        {
            printf("Cannot find file %s", entry->d_name);
        }

        printf("=============================\n");
        printf("File Name: ", entry->d_name);
        printf("File Path: ", file_path);
        printf("=============================\n");


    }
    closedir(dir);
    return 0;
    
}