#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H


#define BLOCK_SIZE_512 512
#define BLOCK_SIZE_1024 1024 
#define NUM_BLOCKS 4096 // 12-bit addressing allows for 2^12 = 4096 blocks
#define FAT_FREE 0x000
#define FAT_END 0xFFF
#include <time.h>
// super block, data blocks, free blocks, directories, data


typedef struct {
    char *fileName; 
    char *path;
    int fileSize;
    char owner_permissions; 
    time_t last_modification;
    time_t creation_time;
    char *password; 
    int first_block; 
} DirectoryEntry;

typedef struct 
{
    int block_size;
    int num_blocks;
    int fat_start;
    int root_dir_start;
    int data_start;
    int bitmap_start;
}SuperBlock;


typedef struct {
    SuperBlock superBlock;
    int FAT[NUM_BLOCKS];
    DirectoryEntry rootDir[NUM_BLOCKS];
    int bitmap[NUM_BLOCKS];
    char* block_storage[NUM_BLOCKS];
} FileSystem;

void init_file_system(FileSystem *fs, int block_size, const char *filename);
void print_super_block(FileSystem *fs);

#endif

