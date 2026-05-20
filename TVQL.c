#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

int main()
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char file_path[1024];

    dir = opendir(".");
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

        if(lstat(file_path, &info) == NULL)
        {
            printf("Cannot find file: /%s", entry->d_name);
            continue;
        }

        printf("======================================\n");
        printf("File name: %s\n", entry->d_name);
        printf("File path: %s\n", file_path);
        printf("======================================\n");

        printf("Device ID: %lu\n", (unsigned long)info.st_dev);
        printf("Inode number (unique File ID): %lu\n",(unsigned long)info.st_ino);
        printf("File type & permissions: %o\n", info.st_mode);
        printf("Number of hard links: %lu\n", (unsigned long)info.st_nlink);
        printf("Owner's User ID: %u\n",info.st_uid);
        printf("Owner's group ID: %u\n",info.st_gid);
        printf("Device ID (if special file): %lu\n",(unsigned long)info.st_rdev);
        printf("File size in Bytes: %ld\n", (long)info.st_size);
        printf("Block size for filesystem I/O: %ld\n",(long)info.st_blksize);
        printf("Number of 512 blocks allocated: %ld\n", (long)info.st_blocks);
        printf("Last access time: %s\n", ctime(&info.st_atime));
        printf("Last modification time: %s\n", ctime(&info.st_mtime));
        printf("Last status change: %s\n", ctime(&info.st_ctime));
        printf("=============================================\n");
    }

    return 0;
}