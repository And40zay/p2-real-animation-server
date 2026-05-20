#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>  // Added for strcmp()

int main()
{
    DIR *dir;
    struct dirent *entry;
    struct stat info;
    char full_path[1024];  // For building full paths
    
    dir = opendir(".");
    if(dir == NULL)
    {
        printf("Error: Cannot open root directory /\n");
        return 1;
    }

    printf("=== COMPLETE FILE INFORMATION FOR ROOT DIRECTORY ===\n\n");

    while((entry = readdir(dir)) != NULL)
    {
        // FIX 1: Compare characters, not strings
        // Was: if(entry->d_name[0] == ".")  // WRONG: char == char*
        if(entry->d_name[0] == '.')  // CORRECT: compare with '.' not "."
        {
            continue;
        }

        // FIX 2: Build full path for root directory
        // lstat("bin") looks for ./bin (current directory), not /bin
        snprintf(full_path, sizeof(full_path), "/%s", entry->d_name);
        
        // Use the full path
        if(lstat(full_path, &info) == -1)
        {
            printf("Cannot access: /%s\n", entry->d_name);
            continue;
        }

        printf("========================================\n");
        printf("File Name: %s\n", entry->d_name);
        printf("Full Path: %s\n", full_path);
        printf("========================================\n");
        
        // Cast all fields properly
        printf("Device ID: %lu\n", (unsigned long)info.st_dev);
        printf("Inode number: %lu\n", (unsigned long)info.st_ino);
        printf("Permissions (octal): %o\n", info.st_mode);
        printf("Number of hard links: %lu\n", (unsigned long)info.st_nlink);
        printf("Owners UserID: %u\n", info.st_uid);
        printf("Owners group ID: %u\n", info.st_gid);
        printf("Device ID (if special file): %lu\n", (unsigned long)info.st_rdev);
        printf("File size (in bytes): %ld\n", (long)info.st_size);
        printf("Block size for filesystem I/O: %ld\n", (long)info.st_blksize);
        printf("Number of 512B Blocks allocated: %ld\n", (long)info.st_blocks);
        printf("Disk space used: %ld bytes\n", (long)info.st_blocks * 512);
        printf("Last access time: %ld\n", (long)info.st_atime);
        printf("Last modification time: %ld\n", (long)info.st_mtime);
        printf("Last status change time: %ld\n", (long)info.st_ctime);
        
        // Human-readable file type
        printf("File Type: ");
        if (S_ISDIR(info.st_mode)) printf("Directory\n");
        else if (S_ISREG(info.st_mode)) printf("Regular File\n");
        else if (S_ISLNK(info.st_mode)) printf("Symbolic Link\n");
        else if (S_ISCHR(info.st_mode)) printf("Character Device\n");
        else if (S_ISBLK(info.st_mode)) printf("Block Device\n");
        else if (S_ISFIFO(info.st_mode)) printf("FIFO/Pipe\n");
        else if (S_ISSOCK(info.st_mode)) printf("Socket\n");
        else printf("Unknown\n");
        
        printf("\n");
    }
    
    closedir(dir);  // Don't forget to close!
    return 0;
}