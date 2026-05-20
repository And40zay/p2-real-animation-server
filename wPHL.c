#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

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

    
    if((lstat(file_path, &info) == -1))
    {
        continue;
    }    
        printf("=============================\n");
        printf("File Name: %s\n", entry->d_name);
        printf("File Path: %s\n", file_path);
        printf("=============================\n");
        printf("Device ID: %lu\n", (unsigned long)info.st_dev);
        printf("Inode Number (Unique File ID): %lu\n", (unsigned long)info.st_ino);
        printf("Mode (file Type and Permissions): %o\n",info.st_mode);
        printf("NLink (Number of hard links): %lu\n", (unsigned long)info.st_nlink);
        printf("Owner's User ID: %u\n", info.st_uid);
        printf("Owner's Group ID: %u\n", info.st_gid);
        printf("Device ID (if special file): %lu\n", (unsigned long)info.st_rdev);
        printf("File Size in bytes: %ld\n", (long)info.st_size);
        printf("Block size for filesystems I/O: %ld\n", (long)info.st_blksize);
        printf("Number of 512 blocks Allocated: %ld\n", (long)info.st_blocks);
        printf("Last access time: %s", ctime(&info.st_atime));
        printf("Last Modification Time: %s", ctime(&info.st_mtime));
        printf("Last status Change Time: %s\n", ctime(&info.st_ctime));

    }
    closedir(dir);
    return 0;


}