#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char file_path[1024];

    dir = opendir("/");
    if(dir == NULL)
    {
        printf("Error: opendir()");
        return 1;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
        {
            continue;
        }

        snprintf(file_path, sizeof(file_path), "/%s", entry->d_name);

        if(lstat(file_path, &info) == -1)
        {
            printf("Error: lstat(file_path, info)");
            continue;
        }
        printf("=============================\n");
        printf("File name: %s\n", entry->d_name);
        printf("FIle path: %s\n", file_path);
        printf("==============================\n");
    }

}