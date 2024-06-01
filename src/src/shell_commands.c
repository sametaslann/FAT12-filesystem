#include "../include/shell_commands.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
extern char* fileSystemName;

void listDirectory(FileSystem *fs) {
    printf("Permissions  Size    Last Modification       Creation Time            Name    Start_Block\n");
    printf("----------  ------  ----------------------  ----------------------  ----------------\n");
    for (int i = 1; i < NUM_BLOCKS; i++) {
        if (fs->rootDir[i].fileName != NULL) {
            char perm = (fs->rootDir[i].owner_permissions == 'r') ? 'r' : ((fs->rootDir[i].owner_permissions == 'w') ? 'w' : '-');
            
            // Initialize the modification and creation time strings
            char last_modification[20] = {0};
            char creation_time[20] = {0};

            // Remove newline from ctime output
            if (fs->rootDir[i].last_modification != 0) {
                strncpy(last_modification, ctime(&fs->rootDir[i].last_modification), 19);
                last_modification[19] = '\0';
            }
            if (fs->rootDir[i].creation_time != 0) {
                strncpy(creation_time, ctime(&fs->rootDir[i].creation_time), 19);
                creation_time[19] = '\0';
            }

            printf("%-10c  %-6d  %-22s  %-22s  %-16s %d\n",
                perm,
                fs->rootDir[i].fileSize,
                last_modification,
                creation_time,
                fs->rootDir[i].fileName,
                fs->rootDir[i].first_block
            );
            if (strcmp(fs->rootDir[i].password, "") != 0) {
                printf("  [Password protected]");
            }
            printf("\n");
            free(fs->rootDir[i].fileName);
            
        }

    }
}

int findFreeBlock(FileSystem **fs){
    for (int i = 1; i < NUM_BLOCKS; i++)
    {
        printf("%d", (*fs)->bitmap[i]);
        if ((*fs)->bitmap[i] == 1) //isFree
        {
            (*fs)->bitmap[i] = 0;
            printf("Free block found at index %d\n", i);
            return i;
        }
    }
    printf("No free blocks found\n");
    return -1;
}

void createDirectory(FileSystem *fs, char *path, char *fileSystemName){
    
    if (path[0] == '/' && path[1] == '\0'){
        printf("Root directory cannot be created\n");
        return;
    }   

    char *token = strtok(path, "/");
    int indexOfCurrentDir = 0;

    int next_entry;
    for (int i = 1; i < NUM_BLOCKS; i++)
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
    if (token == NULL && !indexOfCurrentDir) //in root directory
    {
        fs->rootDir[next_entry].fileSize = 0;
        fs->rootDir[next_entry].owner_permissions = 'r';
        fs->rootDir[next_entry].last_modification = time(NULL);
        fs->rootDir[next_entry].creation_time = time(NULL);
        fs->rootDir[next_entry].first_block = findFreeBlock(&fs);
        fs->FAT[fs->rootDir[next_entry].first_block] = FAT_END;
        fs->bitmap[fs->rootDir[next_entry].first_block] = 0;
        fs->rootDir[next_entry].isDirectory = 1;

        FILE *file = fopen(fileSystemName, "rb+");
        if (file == NULL) {
            printf("Error opening file.\n");
            exit(1);
        }
        // fs->rootDir[i].fileName = malloc(255 * sizeof(char));

        fseek(file, fs->superBlock.bitmap_start, SEEK_SET);
        fwrite(fs->bitmap, sizeof(fs->bitmap), 1, file);
        fwrite(fs->FAT, sizeof(fs->FAT), 1, file);

        fseek(file, fs->superBlock.data_start - HEAP_SIZE, SEEK_SET);

        for (int i = 0; i < HEAP_SIZE; i++)
        {
            char c;
            fread(&c, sizeof(char), 1, file);
            if (c == '\0')
            {
                fseek(file, -1, SEEK_CUR);
                if(i != 0)            
                    fwrite("|", 1, 1, file);
                break;
            }
        }

        fs->rootDir[next_entry].fileName = currentDirName;
        fwrite(currentDirName, strlen(currentDirName), 1, file);

        printf("\n%s\n",fs->rootDir[next_entry].fileName);

        int newPositionOffset = fs->superBlock.root_dir_start + (next_entry * sizeof(DirectoryEntry));
        fseek(file, newPositionOffset, SEEK_SET);
        printf("Location of root directory: %ld\n", ftell(file));

        fwrite(&fs->rootDir[next_entry], sizeof(DirectoryEntry) , 1, file);

        fclose(file);
    }
    
    else if (token != NULL && indexOfCurrentDir) { //in a subdirectory
        
        int currentDirAddress = fs->rootDir[indexOfCurrentDir].first_block;

        FILE *file = fopen(fileSystemName, "rb+");
        if (file == NULL) {
            printf("Error opening file.\n");
            exit(1);
        }

        const int fileSize = fs->rootDir[indexOfCurrentDir].fileSize;
        int num_block_for_heap = HEAP_SIZE / fs->superBlock.block_size;

        DirectoryEntry newDir = {
            .fileName = NULL,
            .fileSize = 0,
            .owner_permissions = 'r',
            .last_modification = time(NULL),
            .creation_time = time(NULL),
            .first_block = findFreeBlock(&fs)
        };


        // 1- burda heapi bulurken geriye doğru gidiyorsun onu değiştirmen lazım 
        // 2- bulduklarını | ' a göre parse edip filenamelere point etmen lazım
        if (fileSize == 0)
        {
            printf("Second level directory\n");
            fseek(file, fs->superBlock.data_start + currentDirAddress * fs->superBlock.block_size , SEEK_SET);
            fwrite(&newDir, sizeof(DirectoryEntry), 1, file);

            for (int i = 0; i < num_block_for_heap; i++)
            {
                fs->FAT[currentDirAddress + i] = findFreeBlock(&fs); 
            }
            fs->FAT[fs->FAT[currentDirAddress + num_block_for_heap - 1]] = FAT_END;

            int iterator = currentDirAddress;
            for (int i = 0; i < NUM_BLOCKS; i++)
            {
                if (fs->FAT[iterator] == -1)
                {
                    int heap_start_point = fs->superBlock.data_start + ((iterator+1) * fs->superBlock.block_size) - HEAP_SIZE;
                    fseek(file, heap_start_point , SEEK_SET);
                    printf("Location of heap: %ld\n", ftell(file));
                    fwrite("elifaskimm", strlen("elifaskimm"), 1, file);
                    fwrite("|", 1, 1, file);
                    break;
                }
                iterator = fs->FAT[currentDirAddress];
                printf("Iterator: %d\n", iterator);
            }
        }

        fseek(file, fs->superBlock.bitmap_start, SEEK_SET);
        fwrite(fs->bitmap, sizeof(fs->bitmap), 1, file);
        fwrite(fs->FAT, sizeof(fs->FAT), 1, file);
        

        fclose(file);
        
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

