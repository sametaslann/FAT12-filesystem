#include <time.h>
#include "file_system.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void init_file_system(FileSystem *fs, const int block_size, const char *filename) {

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

    // Initialize bitmap
    memset(fs->bitmap, 0xFF, sizeof(fs->bitmap));

    // Initialize block storage
    for (int i = 0; i < NUM_BLOCKS; i++) {
        fs->block_storage[i] = (char*)malloc(sizeof(char) * 1024);
        memset(fs->block_storage[i], 0, 1024);
    }

    // Initialize super block
    fs->superBlock.num_blocks = NUM_BLOCKS;
    fs->superBlock.block_size = block_size;
    fs->superBlock.bitmap_start = sizeof(fs->superBlock);
    fs->superBlock.fat_start = fs->superBlock.bitmap_start + sizeof(fs->bitmap);
    fs->superBlock.root_dir_start = fs->superBlock.fat_start + sizeof(fs->FAT);
    fs->superBlock.data_start = fs->superBlock.root_dir_start + sizeof(DirectoryEntry);


    fwrite(&fs->superBlock, sizeof(fs->superBlock), 1, file);
    fwrite(&fs->bitmap, sizeof(fs->bitmap), 1, file);
    fwrite(&fs->FAT, sizeof(fs->FAT), 1, file);
    fwrite(&fs->rootDir, sizeof(fs->rootDir), 1, file);

    int usedBlocks = (sizeof(fs->superBlock) + sizeof(fs->bitmap) + sizeof(fs->rootDir) + sizeof(fs->FAT)) / block_size;
    int blocksToWrite = NUM_BLOCKS - usedBlocks;


    for (int i = 0; i < blocksToWrite-1; i++) {
        fwrite((fs->block_storage[i]) , 1, block_size, file);
    }
    
    fclose(file);

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

