#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    char file_path[1024];
    int count;
    struct stat info;

    dir = opendir("/");
    if(dir == NULL)
    {
        printf("Cannot access: /\n");
        return 1;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] != '.')
        {
            continue;
        }

        snprintf(file_path, sizeof(file_path), "/%s", entry->d_name);

        if(lstat(file_path, &info) == -1)
        {
            printf("Cannot access /%s\n",entry->d_name);
            continue;
        }

        printf("======================================");
        printf("Full path: %s")
}