#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <ctime>
#include <cstdint>
#include <string>
#include <vector>

const int BLOCK_SIZE_512 = 512;
const int BLOCK_SIZE_1024 = 1024;
const int NUM_BLOCKS = 4096; // 12-bit addressing allows for 2^12 = 4096 blocks
const int FAT_FREE = 0x000;
const int FAT_END = 0xFFF;
const int HEAP_SIZE = 2048;
const int PASSWORD_SIZE = 16;

struct DirectoryEntry {
    char *fileName; 
    int fileSize;
    char owner_permissions; 
    time_t last_modification;
    time_t creation_time;
    char password[PASSWORD_SIZE]; 
    int first_block;
    bool isDirectory;
    int file_count;
};

struct SuperBlock {
    int block_size;
    int num_blocks;
    int fat_start;
    int root_dir_start;
    int data_start;
    int bitmap_start;
    int dataBlockNum;
    int total_dir;

};



class FileSystem {
public:
    SuperBlock superBlock;
    int FAT[NUM_BLOCKS];
    DirectoryEntry rootDir[NUM_BLOCKS];
    int bitmap[NUM_BLOCKS];
    std::vector<char*> block_storage;
    char heap[HEAP_SIZE];
    
    


    std::string filePath;
    int block_size;

    FileSystem(std::string filePath, int block_size);
    FileSystem(std::string filePath);

    void init_file_system();
    void mount_file_system();
    void save_filesystem();
    void dumpe2fs();
    void chmod_file(const std::string &filename, char* permissions);
    void write_file(const std::string path_dir, const std::string filename);
    void read_file(const std::string path_dir, const std::string filename, const std::string password);
    void delete_file(const std::string path_dir,  const std::string password);
    void add_password(const std::string path_dir, const std::string password);




    void list_directories(const std::string &dir_path);

    void create_directory(const std::string &dir_path);
    int find_free_block();



    // void parse_filenames(DirectoryEntry **dir, char *heap);
    int find_next_entry(const char* filename);
    std::vector<DirectoryEntry> get_dir_table(DirectoryEntry *parentDir);


    void update_data_block(std::vector <DirectoryEntry> &dirTable, int first_block);
    void find_corr_dir(std::vector<std::string> tokens, int size, std::vector<DirectoryEntry> &currentTable, int &fileIdx, int &last_first_block);

    void find_file(std::vector<std::string> tokens, std::string filename);
    int write_file_content(int first_block, const std::string filename);
    void read_file_content(int first_block, const std::string filename, char permissions);
    void delete_file_blocks(int first_block);
    void delete_dir_blocks(int first_block, std::vector<DirectoryEntry> &currentTable);





    void occupied_blocks(int first_block);
    void recursive_file_search(std::vector<DirectoryEntry> &currentTable);

    void initialize_heap(int iterator);
    void print_super_block();
};

#endif // FILE_SYSTEM_H
