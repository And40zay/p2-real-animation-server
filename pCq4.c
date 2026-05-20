#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char full_path[2024]
    
    dir = opendir("/");
    if(dir == NULL)
    {
        printf("Error: opendir('')");
        return 1;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(entry->d_name[0] == '.')
        {
            continue;
        }
        snprintf(full_path, sizeof(full_path), "/%S", entry->d_name);

        if(lstat(entry->d_name, &info) == -1)
        {
            continue;
        }

        printf("File Name: %s\n", entry->d_name);
        printf("Device ID: %lu\n", info.st_dev);
        printf("Inode number: %lu\n", info.st_ino);
        printf("Permissions: %o\n", info.st_mode);
        printf("Number of hard links: %lu\n", info.st_nlink);
        printf("Owners UserID: %u\n", info.st_uid);
        printf("Owners group ID: %u\n", info.st_gid);
        printf("Device ID (if special file): %lu\n", info.st_rdev);
        printf("File size (in bytes): %ld\n", (long)info.st_size);
        printf("Block size for filesystem I/O: %ld\n", (long)info.st_blksize);
        printf("Number of 512B Blocks allocated: %ld\n", (long)info.st_blocks);
        printf("Last access time: %ld\n", (long)info.st_atime);
        printf("Last modification time: %ld\n", (long)info.st_mtime);
        printf("Last status change time: %ld\n", (long)info.st_ctime);
    }
}