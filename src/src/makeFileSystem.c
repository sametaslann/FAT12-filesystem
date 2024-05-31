#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/file_system.h"


void check_args(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <block_size> <file_name>\n", argv[0]);
        exit(1);
    }
    if (atof(argv[1]) != 0.5 && atof(argv[1]) != 1.0) {
        printf("Block size must be 0.5 or 1 KB\n");
        exit(1);
    }   
}

int main( int argc, char* argv[])
{
    check_args(argc, argv);
    int block_size = atof(argv[1]) * BLOCK_SIZE_1024;

    FileSystem fs;
    init_file_system(&fs, block_size, argv[2]);
    

    return 0;
}

