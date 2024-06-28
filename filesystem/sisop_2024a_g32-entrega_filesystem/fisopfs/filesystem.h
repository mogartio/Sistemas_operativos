// filesystem.h
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define MAX_PATH 50
#define MAX_INDODE 50
#define CONTENT_SIZE 1000


#include <time.h>

#include <sys/types.h>
#include <stdbool.h>

typedef enum inode_type { DIR, DOC } inode_type_t;

typedef struct inode {
	bool is_occupied;
	inode_type_t type;
	char path[MAX_PATH];
	char dir_path[MAX_PATH];
	time_t created_time;
	time_t last_modified_time;
	time_t last_read_time;
	size_t size;
	char content[CONTENT_SIZE];
	uid_t user_id;
	gid_t group_id;
	mode_t mode;

} inode_t;

struct filesystem {
	struct inode inodes[MAX_INDODE];
};

extern struct filesystem filesystem;

#endif
