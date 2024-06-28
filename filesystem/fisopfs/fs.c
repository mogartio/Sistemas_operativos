#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include "filesystem.h"
#define MAX_FILES 100
#define MAX_CONTENIDO_FILE 1000
#define MAX_PATH 50
#define MAX_INDODE 50
#define CONTENT_SIZE 1000


int
fs_init(char path_persistencia[MAX_PATH])
{
	FILE *storage_file_pointer = fopen(path_persistencia, "r");
	if (!storage_file_pointer) {
		return -1;
	}
	int fd_storage = fileno(storage_file_pointer);
	int res = read(fd_storage, &filesystem.inodes, sizeof(filesystem.inodes));
	if (res <= 0) {
		printf("[debug] ERROR AL LEER STORAGE");
		return -1;
	}
	return 0;
}

void
fs_destroy(char path_persistencia[MAX_PATH])
{
	FILE *storage_file_pointer = fopen(path_persistencia, "w");
	if (!storage_file_pointer) {
		printf("[debug] ERROR AL ESCRIBIR STORAGE");
		return;
	}
	int res = fwrite(&filesystem, sizeof(filesystem), 1, storage_file_pointer);
	if (res != 1) {
		printf("[debug] ERROR AL ESCRIBIR STORAGE");
	}

	fflush(storage_file_pointer);
	fclose(storage_file_pointer);
}
// itera el array inodes y devuelve 0 si puede agregar el nuevo inodo. -1 en caso contrario
static int
add_inode(inode_t new_inode)
{
	for (int i = 0; i < sizeof(filesystem.inodes); i++) {
		if (!filesystem.inodes[i].is_occupied) {
			filesystem.inodes[i] = new_inode;
			return 0;
		}
	}
	return -1;
}


int
fs_create_inode(const char *path, mode_t mode, inode_type_t type)
{
	char dir_path[MAX_PATH];
	inode_t new_inode;

	if (type == DIR) {
		strcpy(dir_path, "/");
	} else {
		memcpy(dir_path, path + 1, strlen(path) - 1);
		dir_path[strlen(path) - 1] = '\0';

		char *last_slash = strrchr(dir_path, '/');

		if (last_slash != NULL) {
			*last_slash = '\0';
		} else {
			dir_path[0] = '\0';
		}

		if (strlen(dir_path) == 0) {
			strcpy(dir_path, "/");
		} else {
			char temp_path[MAX_PATH];
			snprintf(temp_path, sizeof(temp_path), "/%s", dir_path);
			strcpy(dir_path, temp_path);
		}

		printf("[debug] dir_path: %s\n", dir_path);
	}

	strcpy(new_inode.path, path);
	strcpy(new_inode.dir_path, dir_path);
	new_inode.size = 0;
	new_inode.created_time = time(NULL);
	new_inode.last_modified_time = time(NULL);
	new_inode.last_read_time = time(NULL);
	new_inode.type = type;
	new_inode.is_occupied = true;
	new_inode.user_id = getuid();
	new_inode.group_id = getgid();
	new_inode.mode = mode;
	return add_inode(new_inode);
}

int
fs_encontrar_indice(const char *path)
{
	for (int i = 0; i < sizeof(filesystem.inodes); i++) {
		if (strcmp(filesystem.inodes[i].path, path) == 0) {
			return i;
		}
	}
	return -1;
}

int
fs_write(const char *path, const char *buf, size_t size, off_t offset)
{
	int indice = fs_encontrar_indice(path);
	if (indice < 0) {
		mode_t mode = __S_IFREG | 0644;
		int res = fs_create_inode(path, mode, DOC);
		if (res == -1) {
			return -1;  //-EFBIG
		}
		indice = fs_encontrar_indice(path);
	}
	if (size + offset > CONTENT_SIZE) {
		return -2;  //-EINVAL;
	}
	inode_t *inode = &filesystem.inodes[indice];
	strncpy(inode->content + offset, buf, size);
	if (inode->size < size + offset) {
		inode->content[size + offset] = '\0';
	}
	inode->size = strlen(inode->content);
	return 0;
}

int
fs_utimens(const char *path, const struct timespec ts[2])
{
	for (int i = 0; i < MAX_INDODE; i++) {
		inode_t *inode = &filesystem.inodes[i];
		if (strcmp(path, inode->path) == 0) {
			inode->last_read_time = ts[0].tv_sec;
			inode->last_modified_time = ts[1].tv_sec;
			return 0;
		}
	}

	return -1;
}

int
fs_rmdir(const char *path)
{
	for (int i = 0; i < MAX_INDODE; i++) {
		inode_t *inode = &filesystem.inodes[i];
		if (inode->type == DIR && strcmp(path, inode->path) == 0) {
			for (int j = 0; j < MAX_INDODE; j++) {
				inode_t *child_inode = &filesystem.inodes[j];
				if (strcmp(child_inode->dir_path, path) == 0) {
					return -1;  //-ENOTEMPTY;
				}
			}

			memset(inode, 0, sizeof(inode_t));
			return 0;
		}
	}

	return -2;  //-ENOENT;
}

int
fs_read(const char *path, char *buffer, size_t size, off_t offset)
{
	for (int i = 0; i < MAX_INDODE; i++) {
		inode_t inode = filesystem.inodes[i];

		if (inode.type == DOC && strcmp(path, inode.path) == 0) {
			if (offset < inode.size) {
				if (offset + size > inode.size)
					size = inode.size - offset;
				memcpy(buffer, inode.content + offset, size);
			} else {
				size = 0;
			}
			return size;
		}
	}

	return -1;  // -ENOENT;
}
