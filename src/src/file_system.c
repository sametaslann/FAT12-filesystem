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
        fs->rootDir[i].password = NULL;
        fs->rootDir[i].fileSize = 0;
        fs->rootDir[i].owner_permissions = 0;
        fs->rootDir[i].last_modification = 0;
        fs->rootDir[i].creation_time = 0;
        fs->rootDir[i].first_block = FAT_FREE;
    }
    printf("Root directory initialized\n");
    // Initialize bitmap
    memset(fs->bitmap, 1, sizeof(fs->bitmap));
    printf("Bitmap initialized\n");

    // Initialize block storage
    for (int i = 0; i < NUM_BLOCKS; i++) {
        fs->block_storage[i] = (char*)malloc(block_size);
        memset(fs->block_storage[i], 0, block_size);
    }
    printf("Block storage initialized\n");
    // Initialize super block
    fs->superBlock.num_blocks = NUM_BLOCKS;
    fs->superBlock.block_size = block_size;
    fs->superBlock.bitmap_start = sizeof(fs->superBlock);
    fs->superBlock.fat_start = fs->superBlock.bitmap_start + sizeof(fs->bitmap);
    fs->superBlock.root_dir_start = fs->superBlock.fat_start + sizeof(fs->FAT);
    fs->superBlock.data_start = fs->superBlock.root_dir_start + sizeof(fs->rootDir) + 1024;


    int usedBlocks = fs->superBlock.data_start / block_size;
    printf("data_start: %d\n", usedBlocks);
    
    
    for (int i = 0; i < usedBlocks; i++)
        fs->bitmap[i] = 0;
    for (int i = usedBlocks; i < NUM_BLOCKS; i++)
        fs->bitmap[i] = 1;

    char heap[1024] = {0};
    printf("%s", heap);
    fwrite(&fs->superBlock, sizeof(fs->superBlock), 1, file);
    fwrite(&fs->bitmap, sizeof(fs->bitmap), 1, file);
    fwrite(&fs->FAT, sizeof(fs->FAT), 1, file);
    fwrite(&fs->rootDir, sizeof(fs->rootDir), 1, file);
    fwrite(heap, 1024 , 1, file);

    usedBlocks = (sizeof(fs->superBlock) + sizeof(fs->bitmap) + sizeof(fs->rootDir)+ (1024) + sizeof(fs->FAT)) / block_size;
    printf("data_start: %d\n", usedBlocks);
    
    fs->superBlock.dataBlockNum = NUM_BLOCKS - usedBlocks;
    printf("blocksToWrite: %d\n", fs->superBlock.dataBlockNum);


    for (int i = 0; i < fs->superBlock.dataBlockNum-1; i++) {
        fwrite((fs->block_storage[i]) , 1, block_size, file);
    }

    
    fclose(file);
    print_super_block(fs);
}


void mount_file_system(FileSystem *fs, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    char *heap = (char*)malloc(sizeof(char) * 1024);
    fread(&fs->superBlock, sizeof(fs->superBlock), 1, file);
    fread(&fs->bitmap, sizeof(fs->bitmap), 1, file);
   
    fread(&fs->FAT, sizeof(fs->FAT), 1, file);
    fread(&fs->rootDir, sizeof(fs->rootDir), 1, file);
    fread(heap, 1024, 1 , file);
    printf("Heap1:%s\n",heap);

    fs->rootDir[0].fileName = heap;

    printf("Heap2:%s\n",fs->rootDir[0].fileName);

    
    for (int i = 0; i < fs->superBlock.dataBlockNum; i++) {
        fs->block_storage[i] = (char*)malloc(sizeof(char) * 1024);
        fread(fs->block_storage[i], 1, 1024, file);
    }
    fclose(file);
    print_super_block(fs);
}



void print_super_block(FileSystem *fs) {
    printf("|------------- Super Block -------------|\n");
    printf("| Block Size: %d    \t\t\t|\n", fs->superBlock.block_size);
    printf("| Number of Blocks: %d   \t\t|\n", fs->superBlock.num_blocks);
    printf("| Bitmap Start: %d   \t\t\t|\n", fs->superBlock.bitmap_start);
    printf("| FAT Start: %d   \t \t\t|\n", fs->superBlock.fat_start);
    printf("| Root Directory Start: %d  \t \t|\n", fs->superBlock.root_dir_start);
    printf("| Data Start: %d   \t \t\t|\n", fs->superBlock.data_start);
    printf("|------------- Super Block -------------|\n");

   
    
}

