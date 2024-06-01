#include <time.h>
#include "../include/file_system.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void init_file_system(FileSystem *fs, const int block_size, const char *filename) {

    printf("Initializing file system..\n");

    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }


    // Initialize FAT
    memset(fs->FAT, -1, sizeof(fs->FAT));


    // Initialize directoryTable
    for (int i = 0; i < NUM_BLOCKS; i++) {
        fs->rootDir[i].fileName = NULL;
        strcpy(fs->rootDir[i].password, "");
        fs->rootDir[i].fileSize = 0;
        fs->rootDir[i].owner_permissions = 0;
        fs->rootDir[i].last_modification = 0;
        fs->rootDir[i].creation_time = 0;
        fs->rootDir[i].first_block = FAT_FREE;
    }
    printf("Root directory initialized %ld\n", sizeof(fs->rootDir));

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        fs->bitmap[i] = 1;
    }
    
    printf("Bitmap initialized\n");

    
    // Initialize super block
    fs->superBlock.num_blocks = NUM_BLOCKS;
    fs->superBlock.block_size = block_size;
    fs->superBlock.bitmap_start = sizeof(fs->superBlock);
    fs->superBlock.fat_start = fs->superBlock.bitmap_start + sizeof(fs->bitmap);
    fs->superBlock.root_dir_start = fs->superBlock.fat_start + sizeof(fs->FAT);
    fs->superBlock.data_start = fs->superBlock.root_dir_start + sizeof(fs->rootDir) + HEAP_SIZE;


    int usedBlocks = fs->superBlock.data_start / block_size;
    printf("data_start: %d\n", usedBlocks);

    // Initialize block storage
    for (int i = 0; i < NUM_BLOCKS; i++) {

        fs->block_storage[i] = (char*)malloc(block_size);
        if (fs->block_storage[i] == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        
        memset(fs->block_storage[i], 0, block_size);
    }
    printf("Block storage initialized\n");
    
    usedBlocks = (sizeof(fs->superBlock) + sizeof(fs->bitmap) + sizeof(fs->rootDir)+ (HEAP_SIZE) + sizeof(fs->FAT)) / block_size;
    fs->superBlock.dataBlockNum = NUM_BLOCKS - usedBlocks;
 

    char heap[HEAP_SIZE];
    memset(heap, 0, HEAP_SIZE);
    fwrite(&fs->superBlock, sizeof(fs->superBlock), 1, file);
    fwrite(&fs->bitmap, sizeof(fs->bitmap), 1, file);
    fwrite(&fs->FAT, sizeof(fs->FAT), 1, file);
    fwrite(&fs->rootDir, sizeof(fs->rootDir), 1, file);
    // fwrite("selam1|", 6, 1, file);
    // fwrite("|", 1, 1, file);
    // fwrite("selam2", 6, 1, file);
    fwrite(heap, HEAP_SIZE , 1, file);

    printf("blocksToWrite: %d\n", fs->superBlock.dataBlockNum);


    for (int i = 0; i < fs->superBlock.dataBlockNum-1; i++) {
        fwrite((fs->block_storage[i]) , 1, block_size, file);
    }

    
    fclose(file);

    //free everything
    for (int i = 0; i < NUM_BLOCKS; i++) {
        free(fs->block_storage[i]);
    }




    print_super_block(fs);
}

void parse_filenames(FileSystem **fs, char *heap) {

    char *token = strtok(heap, "|");
    int i = 1;
    while (token != NULL) {
        (*fs)->rootDir[i].fileName = (char*)malloc(strlen(token) + 1);
        if ((*fs)->rootDir[i].fileName == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        
        strcpy((*fs)->rootDir[i].fileName, token);        
        printf("root dir sub:%s %d\n",(*fs)->rootDir[i].fileName, i);

        token = strtok(NULL, "|");
        i++;
    }
}

void mount_file_system(FileSystem *fs, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    char *heap = (char*)malloc(sizeof(char) * HEAP_SIZE);
    fread(&fs->superBlock, sizeof(fs->superBlock), 1, file);
    fread(&fs->bitmap, sizeof(fs->bitmap), 1, file);
   
    fread(&fs->FAT, sizeof(fs->FAT), 1, file);
    fread(&fs->rootDir, sizeof(fs->rootDir), 1, file);
    fread(heap, HEAP_SIZE, 1 , file);

    // fs->rootDir[0].fileName = heap;
    printf("Heap: %s\n", heap);
    parse_filenames(&fs, heap);  

    if (fs->rootDir[1].fileName == NULL) {
        printf("No files found\n");
    }


    
    for (int i = 0; i < fs->superBlock.dataBlockNum; i++) {
        fs->block_storage[i] = (char*)malloc(sizeof(char) * 1024);
        fread(fs->block_storage[i], 1, 1024, file);
        if (i <50)
        {
            printf("Block storage: %s\n", fs->block_storage[i]);
            
        }
        
    }

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if(fs->rootDir[i].first_block != FAT_FREE){
            int iterator = fs->rootDir[i].first_block;
            while (1)
            {
                printf("Iterator: %d\n", iterator);
                if (fs->FAT[iterator] == -1)
                {
                    break;
                }
                iterator = fs->FAT[iterator];

                
            }
            char *buffer = (char*)malloc(sizeof(char) * 1024);
            int num_block_for_heap = HEAP_SIZE / fs->superBlock.block_size;

            int heap_start_point = fs->superBlock.data_start + ((iterator+1) * fs->superBlock.block_size) - HEAP_SIZE;

            printf("Heap start point: %d\n", heap_start_point);
            printf("Iterator: %d\n", iterator);
            fseek(file, heap_start_point, SEEK_SET);
            fread(buffer, 1, 1024, file);
            printf("BUFFERRR: %s\n", buffer);
            
        }
    }
    

    fclose(file);
    print_super_block(fs);
    free(heap);
}



void print_super_block(FileSystem *fs) {
    printf("|------------- Super Block -------------|\n");
    printf("| Block Size: %d    \t\t\t|\n", fs->superBlock.block_size);
    printf("| Number of Blocks: %d   \t\t|\n", fs->superBlock.num_blocks);
    printf("| Bitmap Start: %d   \t\t\t|\n", fs->superBlock.bitmap_start);
    printf("| FAT Start: %d   \t \t\t|\n", fs->superBlock.fat_start);
    printf("| Root Directory Start: %d  \t \t|\n", fs->superBlock.root_dir_start);
    printf("| Data Start: %d   \t \t\t|\n", fs->superBlock.data_start);
    printf("| Data Block Number: %d   \t \t|\n", fs->superBlock.dataBlockNum);
    printf("|------------- Super Block -------------|\n");

   
    
}

