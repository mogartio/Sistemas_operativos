#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include "fs.h"
#include "filesystem.h"

struct filesystem filesystem;
char path_persistencia[MAX_PATH] = "storage.fisopfs";

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	for (int i = 0; i < MAX_INDODE; i++) {
		inode_t inode = filesystem.inodes[i];
		if (strcmp(path, inode.path) == 0) {
			st->st_size = inode.size;
			st->st_ctime = inode.created_time;
			st->st_mtime = inode.last_modified_time;
			st->st_atime = inode.last_read_time;
			st->st_uid = inode.user_id;
			st->st_gid = inode.group_id;
			st->st_mode = inode.mode;
			st->st_dev = 0;

			if (inode.type == DOC) {
				st->st_mode = __S_IFREG | 0644;
				st->st_nlink = 1;
			} else {
				st->st_nlink = 2;
				st->st_mode = __S_IFDIR | 0755;
			}

			return 0;
		}
	}
	return -ENOENT;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir - path: %s\n", path);

	// Los directorios '.' y '..'
	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	for (int i = 0; i < MAX_INDODE; i++) {
		inode_t inode = filesystem.inodes[i];

		if (inode.type == DIR && strcmp(path, inode.path) == 0) {
			for (int j = 0; j < MAX_INDODE; j++) {
				inode_t child_inode = filesystem.inodes[j];
				if (strcmp(child_inode.dir_path, path) == 0) {
					const char *child_name =
					        strrchr(child_inode.path, '/') + 1;
					filler(buffer, child_name, NULL, 0);
				}
			}
			return 0;
		}
	}
	return -ENOENT;
}

static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);
	int res = fs_read(path, buffer, size, offset);
	if (res == -1) {
		return -ENOENT;
	}
	return res;
}

void
initialize_filesystem()
{
	memset(&filesystem, 0, sizeof(filesystem));

	inode_t root;
	strcpy(root.path, "/");
	strcpy(root.dir_path, "");
	root.size = 0;
	root.created_time = time(NULL);
	root.last_modified_time = time(NULL);
	root.last_read_time = time(NULL);
	root.type = DIR;
	root.is_occupied = true;

	filesystem.inodes[0] = root;
}

void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] Initializing filesystem - storage: %s\n",
	       path_persistencia);

	int res = fs_init(path_persistencia);
	if (res == -1) {
		printf("[debug] Error leyendo el archivo de storage\n");
		initialize_filesystem();
	}
	return NULL;
}


void
fisopfs_destroy(void *private_data)
{
	printf("[debug] Destroying filesystem\n");
	fs_destroy(path_persistencia);
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink - path: %s\n", path);

	for (int i = 0; i < MAX_INDODE; i++) {
		inode_t *inode = &filesystem.inodes[i];
		if (inode->type == DOC && strcmp(path, inode->path) == 0) {
			memset(inode, 0, sizeof(inode_t));
			return 0;
		}
	}

	return -ENOENT;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);
	int res = fs_rmdir(path);
	if (res == -1) {
		return -ENOTEMPTY;
	}
	if (res == -2) {
		return -ENOENT;
	}
	return res;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] Creando directorio - path: %s\n", path);

	int res = fs_create_inode(path, mode, DIR);
	if (res == -1) {
		printf("[debug] error creando directorio\n");
		return -errno;
	}
	return 0;
}

static int
fisopfs_access(const char *path, int mask)
{
	printf("[debug] fisopfs_access - path: %s\n", path);
	return 0;
	for (int i = 0; i < sizeof(filesystem.inodes); i++) {
		if (strcmp(path, filesystem.inodes[i].path) == 0) {
			printf("[debug] encontre el path\n");
			return 0;
		}
	}
	printf("[debug] error no encontre el path\n");
	return -ENOENT;
}

static int
fisopfs_readlink()
{
	printf("[debug] fisopfs_readlink\n");
	return 0;
}

static int
fisopfs_write(const char *path,
              const char *buf,
              size_t size,
              off_t offset,
              struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_write - size : %zu, offset : %ld\n", size, offset);
	int res = fs_write(path, buf, size, offset) == 0;
	if (res == -1) {
		return -EFBIG;
	}
	if (res == -2) {
		return -EINVAL;
	}
	return (int) size;
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s size: %ld\n", path, size);
	int indice = fs_encontrar_indice(path);
	if (indice < 0) {
		return -ENOENT;
	}
	if (size > CONTENT_SIZE) {
		return -EINVAL;
	}
	filesystem.inodes[indice].size = size;
	return 0;
}

static int
fisopfs_opendir()
{
	printf("[debug] fisopfs_opendir \n");
	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	printf("[debug] fisopfs_create - path: %s\n", path);
	fs_create_inode(path, mode, DOC);
	return 0;
}

static int
fisopfs_utimens(const char *path, const struct timespec ts[2])
{
	printf("[debug] fisopfs_utimens - path: %s\n", path);
	int res = fs_utimens(path, ts);
	if (res == -1) {
		return -ENOENT;
	}
	return res;
}


static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.init = fisopfs_init,
	.destroy = fisopfs_destroy,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.mkdir = fisopfs_mkdir,
	.access = fisopfs_access,
	.readlink = fisopfs_readlink,
	.write = fisopfs_write,
	.truncate = fisopfs_truncate,
	.create = fisopfs_create,
	.utimens = fisopfs_utimens,
};


int
main(int argc, char *argv[])
{
	if (argc == 3 && strcmp(argv[1], "-f") != 0) {
		strcpy(path_persistencia, argv[2]);
		argv[2] = NULL;
		argc--;
	}
	if (argc == 4) {
		strcpy(path_persistencia, argv[3]);
		argv[3] = NULL;
		argc--;
	}
	return fuse_main(argc, argv, &operations, NULL);
}
