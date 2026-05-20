#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

const char* get_file_type(unsigned int d_type)
{
    switch(d_type)
    {
        case DT_REG: return "Regular file!";
        case DT_DIR: return "Directory!";
        case DT_LNK: return "SymLink";
        case DT_CHR: return "Char Device";
        case DT_BLK: return "Block device!";
        case DT_FIFO: return "FIFO/Pipe";
        case DT_SOCK: return "Socket";
        case DT_UNKNOWN: return "Unknown";
        default: return "Other";
    }
}


int main()
{
    DIR *dir;
    struct dirent *entry;
    char file_path[1024];
    struct stat info;

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

        snprintf(file_path, sizeof(file_path), "/%s", entry->d_name);

        if(lstat(file_path, &info) == -1)
        {
            printf("Cannot find file: /%s", entry->d_name);
            continue;
        }

        printf("===============================\n");
        printf("File name: %s\n", entry->d_name);
        printf("File path: %s\n", file_path);
        printf("===============================\n");
        printf("File type: %s\n", get_file_type(entry->d_type));
        printf("Device ID: %lu\n", (unsigned long)info.st_dev);
        printf("Inode number (Unique file ID): %lu\n", (unsigned long)info.st_ino);
        printf("File type & permissions: %o\n", (int)info.st_mode);
        printf("Number of hard links: %lu\n", (unsigned long)info.st_nlink);
        printf("Owner's User ID: %u\n", info.st_uid);
        printf("Owner's group ID: %u\n", info.st_gid);
        printf("Device ID (if special file): %lu\n", (unsigned long)info.st_rdev);
        printf("File size in Bytes: %ld\n", (long)info.st_size);
        printf("Block size for file System I/O: %ld\n", (long)info.st_blksize);
        printf("Number of 512B blocks allocated: %ld\n", (long)info.st_blocks);
        printf("Last access time: %s\n", ctime(&info.st_atime));
        printf("Last modification time: %s\n", ctime(&info.st_mtime));
        printf("Last Status change time: %s\n", ctime(&info.st_ctime));





    }

    return 0;

}

