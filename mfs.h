#ifndef MFS_H
#define MFS_H
#include <sys/types.h>
int
storage_read(const char *path, char *buf, size_t size, off_t offset);
int
storage_write(const char *path, const char *buf, size_t size, off_t offset);
int
find_parent(const char *path);
int
mknod(const char *path, int mode);
int
mkdir(const char *path, mode_t mode);
int
mkroot(const char *path, int mode);
int
readdir(const char *path);
int
write_sp(char *data, int inode, int ptr, const char *buf, size_t size);
int
count_l(const char *path);
char*
split(const char *path, int n);
char*
get_data(int offset);
#endif
