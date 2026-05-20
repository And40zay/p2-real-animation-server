#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    return 0;
}

void visit_recursive (const char *base_path, int depth)
{
    DIR *dir = opendir(base_path);
    if(!dir)
    {
        return;
    }
    struct dirent *entry;
    struct stat statbuf;
    char path[1048];

    while(entry = readdir(dir) != NULL)
    {
        if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }
        snprintf(path, sizeof(path), '%s/%s', base_path, entry->d_name);
    }

    for(int i = 0; i<depth; i++)
    {
        printf("  ");
        
    }
}
