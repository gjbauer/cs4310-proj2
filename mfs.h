#ifndef MFS_H
#define MFS_H
int
read(const char *path, char *buf, size_t size, off_t offset);
int
write(const char *path, const char *buf, size_t size, off_t offset);
int
find_parent(const char *path);
int
mknod(const char *path, int mode);
int
mkdir(const char *path, int mode);
int
readdir(const char *path);
int
write_sp(char *data, int inode, int ptr, const char *buf, size_t size);
#endif
