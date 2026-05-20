#include <stdio.h>
#include <dirent.h>

int main()
{
    struct dirent *entry;
    DIR *dir = opendir(".");

    if(dir == NULL)
    {
        perror("error!");
        return 1;
    }
    while((entry == readdir(dir)) != NULL)
    {
        printf("File found: %s\n", entry->d_name);
    }
    closedir(dir);
    return 0;

}