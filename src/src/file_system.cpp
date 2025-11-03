#include <ctime>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream> 
#include <iomanip>
#include <ctime>
#include <sys/stat.h>
#include <sys/types.h>
#include "../include/file_system.hpp"

FileSystem::FileSystem(std::string filePath, int block_size) 
    : filePath(filePath), block_size(block_size){
    block_storage.resize(NUM_BLOCKS);
}
FileSystem::FileSystem(std::string filePath) 
    : filePath(filePath){
    block_storage.resize(NUM_BLOCKS);
}

void FileSystem::init_file_system() {
    printf("Initializing file system..\n");

    FILE *file = fopen(filePath.c_str(), "wb");
    if (file == nullptr) {
        printf("Error opening file.\n");
        exit(1);
    }

    // Initialize FAT
    memset(FAT, -1, sizeof(FAT));

    // Initialize directoryTable
    for (int i = 0; i < NUM_BLOCKS; i++) {
        rootDir[i].fileName = nullptr;
        strcpy(rootDir[i].password, "");
        rootDir[i].fileSize = 0;
        rootDir[i].owner_permissions = 'd';
        rootDir[i].last_modification = 0;
        rootDir[i].creation_time = 0;
        rootDir[i].first_block = FAT_FREE;
        rootDir[i].isDirectory = 0;
    }

    memset(heap, 0, sizeof(heap));

    for (int i = 0; i < NUM_BLOCKS; i++) {
        bitmap[i] = 1;
    }

    
    // Initialize super block
    superBlock.num_blocks = NUM_BLOCKS;
    superBlock.block_size = block_size;
    superBlock.bitmap_start = sizeof(superBlock);
    superBlock.fat_start = superBlock.bitmap_start + sizeof(bitmap);
    superBlock.root_dir_start = superBlock.fat_start + sizeof(FAT);
    superBlock.data_start = superBlock.root_dir_start + sizeof(rootDir) + sizeof(heap);
    superBlock.total_dir = 0;
    
    // Initialize block storage
    for (int i = 0; i < NUM_BLOCKS; i++) {
        block_storage[i] = new char[block_size];
        memset(block_storage[i], 0, block_size);
    }
    
    int usedBlocks = (sizeof(superBlock) + sizeof(bitmap) + sizeof(rootDir)  + sizeof(FAT) + sizeof(heap)) / block_size;
    superBlock.dataBlockNum = NUM_BLOCKS - usedBlocks;

    save_filesystem();
}

void FileSystem::save_filesystem(){

    FILE *file = fopen(filePath.c_str(), "wb");
    if (file == nullptr) {
        printf("Error opening file.\n");
        exit(1);
    }

    fwrite(&superBlock, sizeof(superBlock), 1, file);
    fwrite(bitmap, sizeof(bitmap), 1, file);
    fwrite(FAT, sizeof(FAT), 1, file);
    fwrite(rootDir, sizeof(rootDir), 1, file);
    fwrite(heap, sizeof(heap), 1, file);


    for (int i = 0; i < superBlock.dataBlockNum -1; i++) {
        fwrite(block_storage[i], 1, superBlock.block_size, file);
    }

    fclose(file);
    for (int i = 0; i < NUM_BLOCKS; i++) {
        delete[] block_storage[i];
    }

}

std::vector<std::string> parse_path(const std::string dir_path){
    std::istringstream iss(dir_path);
    std::string token;
    std::vector<std::string> tokens;
    while (std::getline(iss, token, '/')) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    return tokens;
}

void FileSystem::print_super_block() {
    printf("|------------- Super Block -------------|\n");
    printf("| Block Size: %d    \t\t\t|\n", superBlock.block_size);
    printf("| Number of Blocks: %d   \t\t|\n", superBlock.num_blocks);
    printf("| Bitmap Start: %d   \t\t\t|\n", superBlock.bitmap_start);
    printf("| FAT Start: %d   \t \t\t|\n", superBlock.fat_start);
    printf("| Root Directory Start: %d  \t \t|\n", superBlock.root_dir_start);
    printf("| Data Start: %d   \t \t\t|\n", superBlock.data_start);
    printf("| Data Block Number: %d   \t \t|\n", superBlock.dataBlockNum);
    printf("|------------- Super Block -------------|\n");
}


void parse_filenames(std::vector<DirectoryEntry> &dirTable, const char* heap) {
    std::istringstream iss(heap);
    std::string token;
    int i = 0;
    while (std::getline(iss, token, '|')) {
        if (token.empty()) {
            break;
        }
        if ((int)dirTable.size() == i) {
            break;
        }
        
        dirTable[i].fileName = new char[token.length() + 1]; // Allocate memory for fileName
        std::strcpy(dirTable[i].fileName, token.c_str()); // Copy the token into fileName
        i++;
    }
}

void parse_filenames(DirectoryEntry* dirTable, char* heap) {
    std::istringstream iss(heap);
    std::string token;
    int i = 0;
    while (std::getline(iss, token, '|')) {
        if (token.empty()) {
            break;
        }
        dirTable[i].fileName = new char[token.length() + 1]; // Allocate memory for fileName
        std::strcpy(dirTable[i].fileName, token.c_str()); // Copy the token into fileName
        i++;
    }
}


void FileSystem::mount_file_system() {

    FILE *file = fopen(filePath.c_str(), "rb");
    if (file == nullptr) {
        printf("Error opening file.\n");
        exit(1);
    }

    fread(&superBlock, sizeof(superBlock), 1, file);
    fread(bitmap, sizeof(bitmap), 1, file);
    fread(FAT, sizeof(FAT), 1, file);
    fread(rootDir, sizeof(rootDir), 1, file);
    fread(heap, sizeof(heap), 1, file);  

    for (int i = 0; i < superBlock.dataBlockNum - 1; i++) {
        block_storage[i] = new char[1024];
        fread(block_storage[i], 1, 1024, file);
    }  
    // print_super_block();

    parse_filenames(rootDir, heap);
    fclose(file);

}

int FileSystem::find_free_block(){

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (bitmap[i] == 1) //isFree
        {
            bitmap[i] = 0;
            return i;
        }
    }
    printf("No free blocks found\n");
    return -1;
}


void FileSystem::list_directories(const std::string &dir_path) {
    
    std::vector<std::string> tokens = parse_path(dir_path);
    std::vector<DirectoryEntry> currentTable;
    int fileIdx = 0;
    int last_first_block = 0;
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rootDir[i].fileName == nullptr)
            break;
        
        currentTable.push_back(rootDir[i]);
    }

    find_corr_dir(tokens, tokens.size() ,currentTable, fileIdx, last_first_block);


    std::cout << "Permissions  Size    Last Modification       Creation Time            Name    \n";
    std::cout << "----------  ------  ----------------------  ----------------------  ----------------\n";


    for (int i = 0; i < (int)currentTable.size(); i++) {
        if (currentTable[i].fileName != nullptr) {
            
            std::string perm;
            if (currentTable[i].owner_permissions == 'r')
                perm = "r";
            else if (currentTable[i].owner_permissions == 'w')
                perm = "w";
            else if (currentTable[i].owner_permissions == 'd')
                perm = "rw";
            else
                perm = "-";
            
            
            // Initialize the modification and creation time strings
            char last_modification[20] = {0};
            char creation_time[20] = {0};

            // Remove newline from ctime output
            if (currentTable[i].last_modification != 0) {
                strncpy(last_modification, ctime(&currentTable[i].last_modification), 19);
                last_modification[19] = '\0';
            }
            if (currentTable[i].creation_time != 0) {
                strncpy(creation_time, ctime(&currentTable[i].creation_time), 19);
                creation_time[19] = '\0';
            }

            std::cout << std::left << std::setw(10) << perm << "  "
            << std::setw(6) << currentTable[i].fileSize << "  "
            << std::setw(22) << last_modification << "  "
            << std::setw(22) << creation_time << "  "
            << std::setw(16) << currentTable[i].fileName ;
            
            if (std::strcmp(currentTable[i].password, "") != 0) {
                std::cout << "  [Password protected]";
            }
            std::cout << "\n";
            
            delete[] currentTable[i].fileName; // Free the allocated memory for fileName
        }
    }
}

int FileSystem::find_next_entry(const char* filename){
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rootDir[i].fileName == nullptr)
            return i;

        if (strcmp(rootDir[i].fileName, filename) == 0)
            return 0;
    }
    return -1;

}

int check_file_exists(std::string filename, std::vector<DirectoryEntry> rootDir) {
    for (int i = 0; i < (int)rootDir.size(); i++) {
        if (strcmp(rootDir[i].fileName, filename.c_str()) == 0){
            return i;
        }
    }
    return -1;
}
std::vector<DirectoryEntry> FileSystem::get_dir_table(DirectoryEntry *parentDir){

    int num_block_for_heap = HEAP_SIZE / superBlock.block_size;

    std::vector<std::string> heap_block;
    
    int iterator = (parentDir)->first_block;
    while (num_block_for_heap > 0)
    {
        std::string block_data(block_storage[iterator]);
        heap_block.push_back(block_data);

        iterator = FAT[iterator];
        --num_block_for_heap;

        if (num_block_for_heap == 0)
        {
            break;
        }
    }
    std::string heap_str;
    heap_str.append(heap_block[0]);
    heap_str.append(heap_block[1]);

   

    std::vector<char*> curr_dir_table;
    while (iterator != -1 && FAT[iterator] != FAT_END)
    {
        curr_dir_table.push_back(block_storage[iterator]);
        iterator = FAT[iterator];
    }

    int max_dir_in_block = superBlock.block_size / sizeof(DirectoryEntry); 

    std::vector<DirectoryEntry> currentDirEntries;
    for (int i = 0; i < (int)curr_dir_table.size(); i++) {
        for (int j = 0; j < max_dir_in_block; j++)
        {
            DirectoryEntry* dirTable = reinterpret_cast<DirectoryEntry*>(curr_dir_table[i] + j * sizeof(DirectoryEntry));

            if (dirTable->owner_permissions != 'r' && dirTable->owner_permissions != 'w' && dirTable->owner_permissions != 'd')
            {
                i = curr_dir_table.size()+1;
                break;

            }
            
            currentDirEntries.push_back(*dirTable);
        }

    }

    parse_filenames(currentDirEntries, heap_str.c_str());

    return currentDirEntries;
}





void FileSystem::initialize_heap(int iterator){

    int num_block_for_heap = HEAP_SIZE / superBlock.block_size;

    for (int i = 0; i < num_block_for_heap; i++)
    {
        if (i == (num_block_for_heap ))
        {
            FAT[iterator] = -1;
            break;
        }
        else
        {
            FAT[iterator] = find_free_block();
            iterator = FAT[iterator]; 
        }

    }
}

void FileSystem::update_data_block(std::vector <DirectoryEntry> &dirTable, int first_block)
{

    // insert new heap
    int num_block_for_heap = HEAP_SIZE / superBlock.block_size;
    int iterator = first_block;

    std::string heap;
    for (size_t i = 0; i < dirTable.size(); i++) {

        heap.append(dirTable[i].fileName);
        heap.append("|");

    }
    heap.resize(HEAP_SIZE);

    

    iterator = first_block;
    for (int i = 0; i < num_block_for_heap; i++)
    {
        strncpy(block_storage[iterator], heap.c_str() + i * superBlock.block_size , superBlock.block_size);

        if (FAT[iterator] != -1)
        {
            iterator = FAT[iterator]; 
        }
        
    }



    int max_dir_in_block = superBlock.block_size / sizeof(DirectoryEntry); 
    int block_num_dir = dirTable.size() * sizeof(DirectoryEntry) / superBlock.block_size;

    if (block_num_dir == 0)
        block_num_dir = 1;

    
    memset(block_storage[iterator], 0, superBlock.block_size);

    while (block_num_dir > 0)   
    {
        
        for (int i = 0; i < (int)dirTable.size(); i++)
        {

            memcpy(block_storage[iterator] + i * sizeof(DirectoryEntry), &dirTable[i], sizeof(DirectoryEntry));

            // strncpy(block_storage[iterator] + (i * sizeof(DirectoryEntry)) , reinterpret_cast<char*>(&dirTable[i]), superBlock.block_size);
            
            if (i == max_dir_in_block - 1)
            {
                iterator = FAT[iterator];
                memset(block_storage[iterator], 0, superBlock.block_size);
            }
            
        }
        block_num_dir -= 1;
    }
}

void FileSystem::find_corr_dir(std::vector<std::string> tokens, int size, std::vector<DirectoryEntry> &currentTable, int &fileIdx, int &last_first_block){


    if (size == 0)
        return;
    
    for (int i = 0; i < size ; i++)
    {
        fileIdx = check_file_exists(tokens[i], currentTable); // check if file exists

        if (fileIdx == -1)
        {
            std::cerr << "Directory does not exist" << std::endl;
            exit(1);
        }
    
        last_first_block = currentTable[fileIdx].first_block;
        
        if(currentTable[fileIdx].isDirectory)
        {
            currentTable = get_dir_table(&currentTable[fileIdx]);
        }   
    }
}


void FileSystem::find_file(std::vector<std::string> tokens, std::string filename){

    // std::vector<DirectoryEntry> currentTable;
    // for (int i = 0; i < NUM_BLOCKS; i++)
    // {
    //     if (rootDir[i].fileName == nullptr)
    //         break;
        
    //     currentTable.push_back(rootDir[i]);
    // }

    // int fileIdx = 0;
    // int last_first_block = 0;
    // find_corr_dir(tokens, tokens.size(), currentTable, fileIdx, last_first_block);

    // int fileIdx = check_file_exists(filename, currentTable);
    // if (fileIdx == -1) {
    //     std::cerr << "File does not exist" << std::endl;
    //     return;
    // }
    // std::cout << "File found" << std::endl;    
}


void FileSystem::create_directory(const std::string &dir_path) {

    if (dir_path == "/") {
        std::cerr << "Root directory cannot be created" << std::endl;
        return;
    }

    std::vector<std::string> tokens = parse_path(dir_path);
    std::vector<DirectoryEntry> currentTable;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rootDir[i].fileName == nullptr)
            break;
        
        currentTable.push_back(rootDir[i]);
    }
    

    if (tokens.size() == 1) //in root directory
    {
        // find next empty in rootDir

        if (check_file_exists(tokens[0], currentTable) != -1)
        {
            printf("Directory already exists\n");
            return;
        }

        int next_entry = find_next_entry(tokens[0].c_str());
        if (next_entry == -1)
        {
            printf("No space left in root directory\n");
            return;
        }

        

        rootDir[next_entry].fileSize = 0;
        rootDir[next_entry].owner_permissions = 'd';
        rootDir[next_entry].last_modification = time(NULL);
        rootDir[next_entry].creation_time = time(NULL);
        FAT[rootDir[next_entry].first_block] = FAT_END;
        bitmap[rootDir[next_entry].first_block] = 0;
        rootDir[next_entry].isDirectory = 1;
        rootDir[next_entry].file_count = 0;


        rootDir[next_entry].first_block = find_free_block();
        int iterator = rootDir[next_entry].first_block;
        initialize_heap(iterator);
        

        // address of the name location in disk
        std::string heap_str(heap);
        heap_str.append(tokens[0]);
        heap_str.append("|");
        strcpy(heap, heap_str.c_str());

        superBlock.total_dir += 1;
        save_filesystem();
    }
    else {

        // DirectoryEntry *currentTable = rootDir;
        
        int last_first_block = 0;
        int fileIdx = 0;

        find_corr_dir(tokens, tokens.size() -1 ,currentTable, fileIdx, last_first_block);

        // add new directory to the last directory
        DirectoryEntry newDir = DirectoryEntry();
        newDir.fileName = (char*)tokens[tokens.size() - 1].c_str();
        newDir.fileSize = 0;
        newDir.owner_permissions = 'd';
        newDir.last_modification = time(NULL);
        newDir.creation_time = time(NULL);
        newDir.first_block = find_free_block();
        newDir.isDirectory = 1;
        newDir.file_count = 0;
        strcpy(newDir.password, "");

        initialize_heap(newDir.first_block);

        currentTable.push_back(newDir);

        update_data_block(currentTable, last_first_block);

        superBlock.total_dir += 1;
        save_filesystem();
    }

}

void FileSystem::occupied_blocks(int first_block){
    int iterator = first_block;
    while (iterator != -1)
    {
        printf("Block: %d\n", iterator);
        iterator = FAT[iterator];
    }
}

void FileSystem::recursive_file_search(std::vector<DirectoryEntry> &currentTable){

    std::vector<DirectoryEntry> tempTable;

    for (int i = 0; i < (int)currentTable.size(); i++) {
        if (currentTable[i].fileName != nullptr) {
            
            printf("Directory: %s\n", currentTable[i].fileName);
            occupied_blocks(currentTable[i].first_block);

            if (currentTable[i].isDirectory != 0)
                tempTable = get_dir_table(&currentTable[i]);
            recursive_file_search(tempTable);

        }
    }

}

void FileSystem::dumpe2fs(){

    int total_free_blocks = 0;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (bitmap[i] == 1)
        {
            total_free_blocks++;
        }
    }
    
    std::vector<DirectoryEntry> currentTable;
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rootDir[i].fileName == nullptr)
            break;
        
        currentTable.push_back(rootDir[i]);
    }
    recursive_file_search(currentTable);

    printf("|------------- Super Block -------------|\n");
    printf("| Block Size: %d    \t\t\t|\n", superBlock.block_size);
    printf("| Number of Blocks: %d   \t\t|\n", superBlock.num_blocks);
    printf("| Bitmap Start: %d   \t\t\t|\n", superBlock.bitmap_start);
    printf("| FAT Start: %d   \t \t\t|\n", superBlock.fat_start);
    printf("| Root Directory Start: %d  \t \t|\n", superBlock.root_dir_start);
    printf("| Data Start: %d   \t \t\t|\n", superBlock.data_start);
    printf("| Data Block Number: %d   \t \t|\n", superBlock.dataBlockNum);
    printf("| Total Free Blocks: %d   \t \t|\n", total_free_blocks);
    printf("| Total Directories: %d   \t \t|\n", superBlock.total_dir);
    printf("|------------- Super Block -------------|\n");

}

void FileSystem::chmod_file(const std::string &dir_path, char* permission) {

    std::vector<std::string> tokens = parse_path(dir_path);
    std::vector<DirectoryEntry> currentTable;




    if (tokens.size() == 1)
    {
        int i;
        for ( i = 0; i < NUM_BLOCKS; i++)
        {
            if (strcmp(rootDir[i].fileName, tokens[0].c_str()) == 0)
                break;
        } 
        
        if (strcmp(permission, "r") == 0)
            rootDir[i].owner_permissions = 'r';
        else if (strcmp(permission, "w") == 0)
            rootDir[i].owner_permissions = 'w';
        else if (strcmp(permission, "+rw") == 0)
            rootDir[i].owner_permissions = 'd';
        else if (strcmp(permission, "-rw") == 0)
            rootDir[i].owner_permissions = '-';
        else
        {
            std::cerr << "Invalid permission" << std::endl;
            exit(1);
        }

    }
    else{

        for (int i = 0; i < NUM_BLOCKS; i++)
        {
            if (rootDir[i].fileName == nullptr)
                break;
            
            currentTable.push_back(rootDir[i]);
        }
        int last_first_block = 0;
        int fileIdx = 0;

        find_corr_dir(tokens, tokens.size()-1, currentTable, fileIdx, last_first_block);
        fileIdx = check_file_exists(tokens[tokens.size() - 1], currentTable);
        if (fileIdx == -1) {
            std::cerr << "File does not exist 2" << std::endl;
            exit(1);
        }

        if (strcmp(permission, "r") == 0)
            currentTable[fileIdx].owner_permissions = 'r';
        else if (strcmp(permission, "w") == 0)
            currentTable[fileIdx].owner_permissions = 'w';
        else if (strcmp(permission, "rw") == 0)
            currentTable[fileIdx].owner_permissions = 'd';
        else
        {
            std::cerr << "Invalid permission" << std::endl;
            exit(1);
        }
        
        currentTable[fileIdx].last_modification = time(NULL);
        update_data_block(currentTable, last_first_block);
    }
    save_filesystem();
}

void check_password(const std::string password, DirectoryEntry &dirEntry){

    if (strcmp(dirEntry.password, "") == 0)
        return;
    
    if (strcmp(dirEntry.password, password.c_str()) == 0)
        printf("Password correct\n");
    else
    {
        printf("Password incorrect\n");
        exit(1);
    }
}



void FileSystem::write_file(const std::string path_dir, const std::string filename){

    std::vector<std::string> tokens = parse_path(path_dir);
    std::vector<DirectoryEntry> currentTable;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rootDir[i].fileName == nullptr)
            break;
        
        currentTable.push_back(rootDir[i]);
    }


    if(tokens.size() == 1)
    {
        int next_entry = find_next_entry(tokens[0].c_str());
        if (next_entry == -1)
        {
            printf("No space left in root directory\n");
            return;
        }


        rootDir[next_entry].fileSize = 0;
        
        rootDir[next_entry].last_modification = time(NULL);
        rootDir[next_entry].creation_time = time(NULL);
        FAT[rootDir[next_entry].first_block] = FAT_END;
        bitmap[rootDir[next_entry].first_block] = 0;
        rootDir[next_entry].isDirectory = 0;
        rootDir[next_entry].file_count = 0;

        struct stat fileStat;
        if(stat(filename.c_str(), &fileStat) < 0) {
            std::cerr << "Error retrieving file information" << std::endl;
            return;
        }


        if (fileStat.st_mode & S_IRUSR && fileStat.st_mode & S_IWUSR)
            rootDir[next_entry].owner_permissions = 'd';
        else if (fileStat.st_mode & S_IRUSR)
            rootDir[next_entry].owner_permissions = 'r';
        else if (fileStat.st_mode & S_IWUSR)
            rootDir[next_entry].owner_permissions = 'w';
        else
            rootDir[next_entry].owner_permissions = '-';


        rootDir[next_entry].first_block = find_free_block();

        int file_size = write_file_content(rootDir[next_entry].first_block, filename);
        rootDir[next_entry].fileSize = file_size;
        

        // address of the name location in disk
        std::string heap_str(heap);
        heap_str.append(tokens[0]);
        heap_str.append("|");
        strcpy(heap, heap_str.c_str());

        superBlock.total_dir += 1;
        save_filesystem();

    }
    else{
         int last_first_block = 0;
    int fileIdx = 0;

    find_corr_dir(tokens, tokens.size() -1 ,currentTable, fileIdx, last_first_block);

    if (check_file_exists(tokens[tokens.size() - 1], currentTable) != -1)
    {
        printf("File already exists\n");
        return;
    }

    // add new directory to the last directory
    DirectoryEntry newFile = DirectoryEntry();
    newFile.fileName = (char*)tokens[tokens.size() - 1].c_str();
    newFile.fileSize = 0;
    newFile.last_modification = time(NULL);
    newFile.creation_time = time(NULL);
    newFile.first_block = find_free_block();
    newFile.isDirectory = 0;
    newFile.file_count = 0;
    strcpy(newFile.password, "");


    struct stat fileStat;
    if(stat(filename.c_str(), &fileStat) < 0) {
        std::cerr << "Error retrieving file information" << std::endl;
        return;
    }


    if (fileStat.st_mode & S_IRUSR && fileStat.st_mode & S_IWUSR)
        newFile.owner_permissions = 'd';
    else if (fileStat.st_mode & S_IRUSR)
        newFile.owner_permissions = 'r';
    else if (fileStat.st_mode & S_IWUSR)
        newFile.owner_permissions = 'w';
    else
        newFile.owner_permissions = '-';
    


    int file_size = write_file_content(newFile.first_block, filename);
    newFile.fileSize = file_size;

    currentTable.push_back(newFile);
    update_data_block(currentTable, last_first_block);
    
    superBlock.total_dir += 1;

    save_filesystem();

    }
}

int FileSystem::write_file_content(int first_block, const std::string filename){
    
    FILE *file = fopen(filename.c_str(), "r");
    if (file == nullptr) {
        std::cerr << "Error opening file" << std::endl;
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);


    int num_blocks = file_size / superBlock.block_size;
    if (file_size % superBlock.block_size != 0) {
        num_blocks++;
    }

    int iterator = first_block;
    for (int i = 0; i < num_blocks; i++) {
        fread(block_storage[iterator], 1, superBlock.block_size, file);
        if (i == num_blocks - 1) {
            FAT[iterator] = -1;
            break;
        } else {
            FAT[iterator] = find_free_block();
        }
        iterator = FAT[iterator];
    }


    fclose(file);
    return file_size;
}


void FileSystem::read_file(const std::string path_dir, const std::string filename, const std::string password) {
    std::vector<std::string> tokens = parse_path(path_dir);
    std::vector<DirectoryEntry> currentTable;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rootDir[i].fileName == nullptr)
            break;
        
        currentTable.push_back(rootDir[i]);
    }

    if(tokens.size() == 1)
    {
        int fileIdx = check_file_exists(tokens[0], currentTable);
        if (fileIdx == -1) {
            std::cerr << "File does not exist" << std::endl;
            return;
        }
        if (currentTable[fileIdx].owner_permissions != 'r' && currentTable[fileIdx].owner_permissions != 'd')
        {
            std::cerr << "Permission denied" << std::endl;
            return;
        }
        check_password(password, currentTable[fileIdx]);
        read_file_content(currentTable[fileIdx].first_block, filename, currentTable[fileIdx].owner_permissions);
        printf("%d bytes read\n", currentTable[fileIdx].fileSize);

    }
    else {
        int last_first_block = 0;
        int fileIdx = 0;

        find_corr_dir(tokens, tokens.size() -1 ,currentTable, fileIdx, last_first_block);

        fileIdx = check_file_exists(tokens[tokens.size() - 1], currentTable);

        if (fileIdx == -1) {
            std::cerr << "File does not exist" << std::endl;
            return;
        }
        if (currentTable[fileIdx].owner_permissions != 'r' && currentTable[fileIdx].owner_permissions != 'd')
        {
            std::cerr << "Permission denied" << std::endl;
            return;
        }
        check_password(password, currentTable[fileIdx]);
        read_file_content(currentTable[fileIdx].first_block, filename, currentTable[fileIdx].owner_permissions);
        printf("%d bytes read\n", currentTable[fileIdx].fileSize);

    }

    
    
}

void FileSystem::read_file_content(int first_block, const std::string filename, char permissions){

    FILE *file = fopen(filename.c_str(), "w");

    if (file == nullptr) {
        std::cerr << "Error opening file" << std::endl;
        exit(1);
    }


    int iterator = first_block;
    while (FAT[iterator] != -1) {
        fwrite(block_storage[iterator], 1, superBlock.block_size, file);
        iterator = FAT[iterator];
    }
    fwrite(block_storage[iterator], 1, superBlock.block_size, file);


    mode_t newPermissions;
    if (permissions == 'd')
        newPermissions = S_IRUSR | S_IWUSR;
    else if (permissions == 'r')
        newPermissions = S_IRUSR;
    else if (permissions == 'w')
        newPermissions = S_IWUSR;    
    
    

    if(chmod(filename.c_str(), newPermissions) < 0) {
        std::cerr << "Error setting file permissions" << std::endl;
        return;
    }
    fclose(file);

}

void FileSystem::delete_file(const std::string path_dir, const std::string password) {

  

    std::vector<std::string> tokens = parse_path(path_dir);
    std::vector<DirectoryEntry> currentTable;

    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rootDir[i].fileName == nullptr)
            break;
        
        currentTable.push_back(rootDir[i]);
    }

    int last_first_block = 0;
    int fileIdx = 0;

    find_corr_dir(tokens, tokens.size() -1 ,currentTable, fileIdx, last_first_block);

    fileIdx = check_file_exists(tokens[tokens.size() - 1], currentTable);

    if (fileIdx == -1) {
        std::cerr << "File does not exist" << std::endl;
        return;
    }
    if (currentTable[fileIdx].owner_permissions != 'w' && currentTable[fileIdx].owner_permissions != 'd')
    {
        std::cerr << "Permission denied" << std::endl;
        return;
    }
    check_password(password, currentTable[fileIdx]);


    //remove from currentTable
    delete_file_blocks(currentTable[fileIdx].first_block);

    currentTable.erase(currentTable.begin() + fileIdx);

    update_data_block(currentTable, last_first_block);
    superBlock.total_dir -= 1;
    save_filesystem();
}

void FileSystem::delete_file_blocks(int first_block){
    int iterator = first_block;
    while (FAT[iterator] != -1) {
        int next_iterator = FAT[iterator];
        bitmap[iterator] = 1;
        FAT[iterator] = -1;
        iterator = next_iterator;
    }
    bitmap[iterator] = 1;
    FAT[iterator] = -1;
}

void FileSystem::add_password(const std::string path_dir, const std::string password){

    if (password.size() > 8)
    {
        std::cerr << "Password too long" << std::endl;
        return;
    }
    

    std::vector<std::string> tokens = parse_path(path_dir);
    std::vector<DirectoryEntry> currentTable;

   
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (rootDir[i].fileName == nullptr)
            break;
        
        currentTable.push_back(rootDir[i]);
    }

    if(tokens.size() == 1)
    {
        int fileIdx = check_file_exists(tokens[0], currentTable);
        if (fileIdx == -1) {
            std::cerr << "File does not exist" << std::endl;
            return;
        }
        if (currentTable[fileIdx].owner_permissions != 'r' && currentTable[fileIdx].owner_permissions != 'd')
        {
            std::cerr << "Permission denied" << std::endl;
            return;
        }

        rootDir[fileIdx].last_modification = time(NULL);
        strcpy(rootDir[fileIdx].password, password.c_str());
    }
    else {
        
        int last_first_block = 0;
        int fileIdx = 0;

        find_corr_dir(tokens, tokens.size() -1 ,currentTable, fileIdx, last_first_block);

        fileIdx = check_file_exists(tokens[tokens.size() - 1], currentTable);

        if (fileIdx == -1) {
            std::cerr << "File does not exist" << std::endl;
            return;
        }
        if (currentTable[fileIdx].owner_permissions != 'r' && currentTable[fileIdx].owner_permissions != 'd')
        {
            std::cerr << "Permission denied" << std::endl;
            return;
        }
        currentTable[fileIdx].last_modification = time(NULL);
        strcpy(currentTable[fileIdx].password, password.c_str());
        update_data_block(currentTable, last_first_block);
    }
    save_filesystem();
}
