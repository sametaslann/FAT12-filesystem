#include <iostream>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include "./include/file_system.hpp"

void check_args(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <block_size> <file_name>" << std::endl;
        exit(1);
    }
    if (std::atof(argv[1]) != 0.5 && std::atof(argv[1]) != 1.0) {
        std::cerr << "Block size must be 0.5 or 1 KB" << std::endl;
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    check_args(argc, argv);
    int block_size = std::atof(argv[1]) * BLOCK_SIZE_1024;


    std::string file_path(argv[2]);
    FileSystem fs(file_path, block_size);
    fs.init_file_system();

    return 0;
}
