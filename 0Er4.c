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
        printf("File Name: %s\n", entry->d_name);
        printf("File Path: %s\n", file_path);
        printf("=============================\n");

        printf("Device ID: %lu\n", (unsigned long)info.st_dev);
        printf("Inode Number: %lu\n", (unsigned long)info.st_ino);
        printf("File Type & Permissions: %o\n", info.st_mode);
        printf("Number of hard links: %lu\n", (unsigned long)info.st_nlink);
        printf("Owners User ID: %u\n", info.st_uid);
        printf("Owners Group ID: %u\n", info.st_gid);
        printf("Device ID (if special file): %lu\n", (unsigned long)info.st_rdev);
        printf("File Size in bytes: %ld\n",(long)info.st_size);
        printf("Block size for filesystem I/O %ld\n", (long)info.st_blksize);
        printf("Number of 512 blocks allocated: %ld\n", (long)info.st_blocks);
a        printf("Last access time: %s\n", ctime((long)info.st_atime));
        printf("Last modification time: %s\n", ctime((long)(info.st_mtime)));
        printf("Last status change time: %s\n", ctime((long)info.st_ctime));


    }
    closedir(dir);
    return 0;
    
}