#ifndef SHELL_H
#define SHELL_H
#include "file_system.h"

void listDirectory(FileSystem *fs);
void createDirectory(FileSystem *fs, char *path,  char *fileSystemName);

// void rmdir(FileSystem *fs, const char *path);
// void dumpe2fs(FileSystem *fs);
// void write(FileSystem *fs, const char *path, const char *linuxFile);
// void read(FileSystem *fs, const char *path, const char *linuxFile);
// void del(FileSystem *fs, const char *path);
// void chmod(FileSystem *fs, const char *path, const char *permissions);
// void addpw(FileSystem *fs, const char *path, const char *password);


#endif