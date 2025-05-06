#include <fuse.h>
int nufs_read(const char *, char *, size_t, off_t, struct fuse_file_info *);
int nufs_write(const char *, const char *, size_t, off_t, struct fuse_file_info *);
int nufs_mkdir(const char *path, mode_t mode);
int nufs_mknod(const char *path, mode_t mode, dev_t rdev);
