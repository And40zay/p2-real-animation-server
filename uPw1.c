#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, int *argv[])
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char file_path[1024];

    const char* directory = ".";
    if(argc>1)
    {
        directory = argv[1];
    }

    dir = opendir("/");
    if(dir == NULL)
    {
        printf("error: opendir()");
        return 1;
    }
    while((entry = readdir(directory)) != NULL)
    {
        if(entry->d_name[0] == "/%s");
        {
            continue;
        }
        snprintf(file_path, sizeof(file_path), "%s%s", directory,  entry->d_name);

        if(lstat(file_path, &info) == -1)
        {
            printf("error: lstat");
            continue;
        }

        printf("==============================\n");
        printf("File NAme: %s\n", entry->d_name);
        printf("File path: %s\n", file_path);
        printf("==============================\n");
    }
}