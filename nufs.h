#ifndef NUFS_H
#define NUFS_H

#define FUSE_USE_VERSION 26
#include <fuse.h>

int
nufs_access(const char *path, int mask);

int
nufs_getattr(const char *path, struct stat *st);

int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi);

int
nufs_mknod(const char *path, mode_t mode, dev_t rdev);

int
nufs_mkdir(const char *path, mode_t mode);

int
nufs_unlink(const char *path);

int
nufs_link(const char *from, const char *to);

int
nufs_rmdir(const char *path);

int
nufs_rename(const char *from, const char *to);

int
nufs_chmod(const char *path, mode_t mode);

int
nufs_truncate(const char *path, off_t size);

int
nufs_open(const char *path, struct fuse_file_info *fi);

int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);

int
nufs_utimens(const char* path, const struct timespec ts[2]);

int
nufs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi,
		   unsigned int flags, void* data);

#endif
