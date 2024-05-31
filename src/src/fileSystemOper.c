#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/file_system.h"
#include "../include/shell_commands.h"



int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <fileSystemName> <operation> [parameters...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *fileSystemName = argv[1];
    char *operation = argv[2];

    FileSystem fs;
    mount_file_system(&fs, fileSystemName);

   
    

    if (strcmp(operation, "dir") == 0) {
        listDirectory(&fs);
    } 
    else if (strcmp(operation, "mkdir") == 0) {
        createDirectory(&fs, argv[3], fileSystemName);
    }
    //  else if (strcmp(operation, "rmdir") == 0) {
    //     removeDirectory(&fs, argv[3]);
    // } else if (strcmp(operation, "dumpe2fs") == 0) {
    //     dumpFileSystemInfo(&fs);
    // } else if (strcmp(operation, "write") == 0) {
    //     writeFile(&fs, argv[3], argv[4]);
    // } else if (strcmp(operation, "read") == 0) {
    //     readFile(&fs, argv[3], argv[4]);
    // } else if (strcmp(operation, "del") == 0) {
    //     deleteFile(&fs, argv[3]);
    // } else if (strcmp(operation, "chmod") == 0) {
    //     changePermissions(&fs, argv[3], argv[4][0]);
    // } else if (strcmp(operation, "addpw") == 0) {
    //     addPassword(&fs, argv[3], argv[4]);
    // } else {
    //     fprintf(stderr, "Unknown operation: %s\n", operation);
    //     return EXIT_FAILURE;
    // }

    return EXIT_SUCCESS;
}

