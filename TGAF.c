#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char* argv[])
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char file_path[1024];
    const char* directory;

    if(argc > 1)
    {
        directory = argv[1];
    }
    
    dir = opendir(directory);
    if(dir == NULL)
    {
        printf("error: opendir(directory)");
        return 1;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
        {
            continue;
        }

        snprintf(file_path, sizeof(file_path), "%s/%s", directory, entry->d_name);

        if(lstat(file_path, &info) == -1)
        {
            continue;
        }

        printf("===============================\n");
        printf("File Name: %s\n", entry->d_name);
        printf("File Path: %s\n", file_path);
        printf("================================\n");

    }

}
