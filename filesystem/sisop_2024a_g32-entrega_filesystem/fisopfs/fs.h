#include "filesystem.h"
#include <stdio.h>
struct File;
extern struct File *files[];
#define MAX_FILES
extern int fs_init(char path_persistencia[30]);


extern struct filesystem filesystem;

extern void fs_destroy(char path_persistencia[30]);

extern int add_inode(inode_t inode);

extern int fs_create_inode(const char *path, mode_t mode, inode_type_t type);

extern int fs_encontrar_indice(const char *path);

extern int fs_write(const char *path, const char *buf, size_t size, off_t offset);

extern int fs_read(const char *path, char *buffer, size_t size, off_t offset);

extern int fs_rmdir(const char *path);

extern int fs_utimens(const char *path, const struct timespec ts[2]);
