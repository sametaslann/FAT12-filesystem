#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "./include/file_system.hpp"

void remove_thicks(char *str) {
    int i = 0;
    int len = strlen(str);
    
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        for (i = 0; i < len - 2; i++) {
            str[i] = str[i + 1];
        }
        str[i] = '\0';
    }
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <fileSystemName> <operation> [parameters...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    std::string file_path(argv[1]);
    char *operation = argv[2];

    
    
    FileSystem fs(file_path);
    
    fs.mount_file_system();

   
    if (strcmp(operation, "dir") == 0) {
        if (argc < 4)
        {
            fprintf(stderr, "Usage: %s <fileSystemName> dir <path> \n", argv[0]);
            return EXIT_FAILURE;
        }
        remove_thicks(argv[3]);
        fs.list_directories(argv[3]);
    } 
    else if (strcmp(operation, "mkdir") == 0) {
        remove_thicks(argv[3]);
        fs.create_directory(argv[3]);
    }
    else if (strcmp(operation, "dumpe2fs") == 0) {
        fs.dumpe2fs();
    }
    else if (strcmp(operation, "chmod") == 0) {
        if (argc < 5)
        {
            fprintf(stderr, "Usage: %s <fileSystemName> chmod <path> <permissions>\n", argv[0]);
            return EXIT_FAILURE;
        }
        remove_thicks(argv[3]);

        fs.chmod_file(argv[3], argv[4]);
    }
    else if (strcmp(operation, "write") == 0) {
        if (argc < 5)
        {
            fprintf(stderr, "Usage: %s <fileSystemName> write <path> <filename>\n", argv[0]);
            return EXIT_FAILURE;
        }
        remove_thicks(argv[3]);
        
        fs.write_file(argv[3], argv[4]);
    }
    else if (strcmp(operation, "read") == 0) {
        if (argc < 5)
        {
            fprintf(stderr, "Usage: %s <fileSystemName> read <path> <filename>\n", argv[0]);
            return EXIT_FAILURE;
        }

        std::string password = "";
        if (argc == 6)
            password = argv[5];
                
        remove_thicks(argv[3]);

        fs.read_file(argv[3], argv[4], password);
    }

    else if (strcmp(operation, "del") == 0) {
        if (argc < 4)
        {
            fprintf(stderr, "Usage: %s <fileSystemName> read <path> \n", argv[0]);
            return EXIT_FAILURE;
        }
        std::string password = "";
        if (argc == 5)
            password = argv[5];
        
        remove_thicks(argv[3]);

        fs.delete_file(argv[3], password);
    }

    else if (strcmp(operation, "addpw") == 0) {
        if (argc < 4)
        {
            fprintf(stderr, "Usage: %s <fileSystemName> addpw <path> <password>\n", argv[0]);
            return EXIT_FAILURE;
        }
        remove_thicks(argv[3]);
        fs.add_password(argv[3], argv[4]);
    }

    else {
        fprintf(stderr, "Unknown operation: %s\n", operation);
        return EXIT_FAILURE;
    }
  

    return EXIT_SUCCESS;
}

