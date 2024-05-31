#include "../include/shell_commands.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
extern char* fileSystemName;

void listDirectory(FileSystem *fs){

    for (int i = 0; i < NUM_BLOCKS; i++) {
        // printf("fs->rootDir[i].first_block %d\n", fs->rootDir[i].first_block);
        if (fs->rootDir[i].first_block != FAT_FREE) {
            // printf("%s\n", fs->rootDir[i].fileName);
            printf("Permissions  Size    Last Modification       Creation Time            Name\n");
            printf("----------  ------  ----------------------  ----------------------  ----------------\n");

            char perm = (fs->rootDir[i].owner_permissions == 'r') ? 'r' : ((fs->rootDir[i].owner_permissions == 'w') ? 'w' : '-');
            // Remove newline from ctime output
            char last_modification[20];
            strncpy(last_modification, ctime(&fs->rootDir[i].last_modification), 19);
            last_modification[20] = '\0';
            char creation_time[20];
            strncpy(creation_time, ctime(&fs->rootDir[i].creation_time), 19);
            creation_time[20] = '\0';

            printf("%-10c  %-6d  %-22s  %-22s  %-16s",
                perm,
                fs->rootDir[i].fileSize,
                last_modification,
                creation_time,
                fs->rootDir[i].fileName
            );
            if (fs->rootDir[i].password != NULL) {
                printf("  [Password protected]");
            }
            printf("\n");
        }
}
}

int findFreeBlock(FileSystem *fs){
    for (int i = 1; i < NUM_BLOCKS; i++)
    {
        if (fs->bitmap[i] == FAT_FREE)
        {
            printf("Free block found at index %d\n", i);
            return i;
        }
    }
    printf("No free blocks found\n");
}

void createDirectory(FileSystem *fs, char *path, char *fileSystemName){
    
    
    if (path[0] == '/' && path[1] == '\0'){
        printf("Root directory cannot be created\n");
        return;
    }   

    char *token = strtok(path, "/");
    int indexOfCurrentDir = 0;

    int next_entry;
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (fs->rootDir[i].fileName == NULL)
        {
            next_entry = i;
            break;
        }
        
        else if (strcmp(fs->rootDir[i].fileName, token) == 0)
        {
            printf("Directory already exists\n");
            indexOfCurrentDir = i;
            break;
        }
    }
    char* currentDirName = token;
    token = strtok(NULL, "/");
    printf("token 2. %s\n", token);
    if (token == NULL && !indexOfCurrentDir)
    {
        fs->rootDir[next_entry].fileSize = 0;
        fs->rootDir[next_entry].owner_permissions = 'r';
        fs->rootDir[next_entry].last_modification = time(NULL);
        fs->rootDir[next_entry].creation_time = time(NULL);
        fs->rootDir[next_entry].first_block = findFreeBlock(fs);
        fs->FAT[fs->rootDir[next_entry].first_block] = FAT_END;

        FILE *file = fopen(fileSystemName, "rb+");
        if (file == NULL) {
            printf("Error opening file.\n");
            exit(1);
        }
        // fs->rootDir[i].fileName = malloc(255 * sizeof(char));
        
        printf("root_dir_start+ sizeof(fs->rootDir) %ld", fs->superBlock.data_start-1024);
        fseek(file, fs->superBlock.data_start-1024, SEEK_SET);
        fwrite(currentDirName, strlen(currentDirName), 1, file);
        printf("token size: %ld\n", strlen(currentDirName));

        fs->rootDir[next_entry].fileName = (char*)(fs->superBlock.root_dir_start+ sizeof(fs->rootDir));
        printf("%d\n",fs->rootDir[next_entry].fileName);

        fseek(file, fs->superBlock.root_dir_start, SEEK_SET);
        printf("Location of root directory: %ld\n", ftell(file));
        fwrite(&token, sizeof(DirectoryEntry), 1, file);
        fwrite(&fs->rootDir[next_entry], sizeof(DirectoryEntry) , 1, file);
    }
    
    else if (token != NULL && indexOfCurrentDir) {
        
        int currentDirAddress = fs->rootDir[indexOfCurrentDir].first_block;
        // int next = fs->FAT[currentDirAddress];

        FILE *file = fopen(fileSystemName, "rb+");
        if (file == NULL) {
            printf("Error opening file.\n");
            exit(1);
        }

        const int fileSize = fs->rootDir[indexOfCurrentDir].fileSize;
        int offsetOfNewDir = (fileSize) * sizeof(DirectoryEntry);
        
        
        DirectoryEntry newDir = {
            .fileName = token,
            .fileSize = 0,
            .owner_permissions = 'r',
            .last_modification = time(NULL),
            .creation_time = time(NULL),
            .first_block = findFreeBlock(fs)
        };
        fs->FAT[newDir.first_block] = FAT_END;

        printf("currentDirAddress: %d\n", currentDirAddress);
        printf("offsetOfNewDir: %d\n", offsetOfNewDir);
        printf("newDir.first_block: %d\n", newDir.first_block);

        fseek(file, currentDirAddress + offsetOfNewDir, SEEK_SET);
        fwrite(&newDir, sizeof(DirectoryEntry), 1, file);

        
    }
    
    





    
    // fs->block_storage[i]  filename buraya alınabilir hayırlısı :((

    // for (int i = 0; i < 100; i++) {
    //     free(dirs[i]);
    
    }    

// void rmdir(FileSystem *fs, const char *path);
// void dumpe2fs(FileSystem *fs);
// void write(FileSystem *fs, const char *path, const char *linuxFile);
// void read(FileSystem *fs, const char *path, const char *linuxFile);
// void del(FileSystem *fs, const char *path);
// void chmod(FileSystem *fs, const char *path, const char *permissions);
// void addpw(FileSystem *fs, const char *path, const char *password);

