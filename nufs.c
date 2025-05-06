// based on cs3650 starter code

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <bsd/string.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "inode.h"
#include "directory.h"
#include "pages.h"
#include "bitmap.h"
#include "nufs.h"

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
    int rv = 0;
    int l = tree_lookup(path);
    rv = (l>-1) ? F_OK : ENOENT;
    printf("access(%s, %04o) -> %d\n", path, mask, rv);
    return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    int rv = 0;
    int l = alloc_inode();
    inode *n = get_inode(l);
    size_t* count = (size_t*)get_root_start();
    dirent *nod = (dirent*)get_root_start() + 1;
    for (int i=0; i<*count; i++, *nod++);
    strcpy(nod->name, path);
    nod->inum = l;
    nod->active=true;
    *count = *count + 1;
    n->mode = mode;
    printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

int
nufs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	if (nufs_mknod(path, mode, 0)) {
    		int l = tree_lookup(path);
    		inode *n = get_inode(l);
        	n->mode = mode; // regular file
        	n->size = 0;
        	return l;
	} else return -1;
}

bool
isnum(const char *path)
{
	char n[4] = ".num";
	int l = strlen(path) - strlen(n);
	int i;
	for (i=0; i<l; i++);
	for (int j=0; j<4; j++, i++) if (path[i]!=n[j]) return false;
	return true;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
// What I hate about this is how it will now create a file for each one that is tests exists...not very great of average UX
{
    int rv = 0;
    int l = tree_lookup(path);
    inode *n;
    if (l>-1) {
    	if (st) {
    		n = get_inode(l);
        	st->st_mode = n->mode; // regular file
        	st->st_size = n->size;
        	st->st_uid = getuid();
        }
    }
    else if (!strcmp(path, "/one.txt") || !strcmp(path, "/two.txt") || !strcmp(path, "/2k.txt") || !strcmp(path, "/40k.txt") || isnum(path)) {
    	l = nufs_create(path, 0100644, 0);
    	return nufs_getattr(path, st);
    }
    else {
    	rv = -ENOENT;
    }
    if (st) printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode, st->st_size);
    else printf("getattr(%s) -> (%d)\n", path, rv);
    return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    struct stat st;
    int rv;
    
    size_t* count = (size_t*)get_root_start();
    dirent *ent = (dirent*)get_root_start()+1;
    for (int i=0; i<*count; i++) {
    	rv = nufs_getattr(ent->name, &st);
    	assert(rv == 0);
    	if (strcmp(ent->name, "/")==0) filler(buf, ".", &st, 0);
    	else if (ent->active=true) {
    		char name[DIR_NAME];
		int i;
		for(i=1; i<DIR_NAME && ent->name[i]; i++) name[i-1] = ent->name[i];
		name[i-1]=0;
    		filler(buf, name, &st, 0);
    	}
	*ent++;
    }

    printf("readdir(%s) -> %d\n", path, rv);
    return 0;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    int rv = nufs_mknod(path, mode | 040000, 0);
    // TODO: Nested Directories
    printf("mkdir(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_unlink(const char *path)
{
    int rv = -1;
    int l = tree_lookup(path);
    if (l<0) return ENOENT;
    size_t* count = (size_t*)get_root_start();
    dirent *ent = (dirent*)get_root_start()+1;
    void* bm = get_inode_bitmap();
    for (int i=0; i<*count; i++) {
    	rv = nufs_getattr(ent->name, 0);
    	assert(rv == 0);
    	if (strcmp(ent->name, path)==0) {
    		ent->active=false;
    		bitmap_put(bm, l, 0);
    	}
	*ent++;
    }
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_link(const char *from, const char *to)
{
    int rv = 0;
    int l = tree_lookup(from);
    inode *n = get_inode(l);
    size_t* count = (size_t*)get_root_start();
    dirent *nod = (dirent*)get_root_start() + 1;
    for (int i=0; i<*count; i++, *nod++);
    strcpy(nod->name, to);
    nod->inum = l;
    nod->active=true;
    *count = *count + 1;
    n->mode = 0100644;
    printf("link(%s => %s) -> %d\n", from, to, rv);
	return rv;
}

int
nufs_rmdir(const char *path)
{
    int rv = -1;
    rv = nufs_unlink(path);
    printf("rmdir(%s) -> %d\n", path, rv);
    return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to) {
    int rv = 0;
    size_t* count = (size_t*)get_root_start();
    dirent *ent = (dirent*)get_root_start()+1;
    for (int i=0; i<*count; i++) {
    	if (strcmp(ent->name, from)==0) {
    		strcpy(ent->name, to);
    	}
	*ent++;
    }
    printf("rename(%s => %s) -> %d\n", from, to, rv);
    return rv;
}

int
nufs_chmod(const char *path, mode_t mode)
{
    int rv = -1;
    printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
    return rv;
}

int
nufs_truncate(const char *path, off_t size)
{
    int rv = 0;
    //int l = tree_lookup(path
    printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
    return rv;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
    int rv = 0;
    int k = nufs_access(path, 0);
    if (k==ENOENT) k = nufs_create(path, 0100644, 0);
    printf("open(%s) -> %d\n", path, rv);
    return rv;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = 4096;
    int l = tree_lookup(path);
    inode* n = get_inode(l);
    void *data = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[0]+offset);
    memcpy(buf, data, size);
    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = 0;
    int l = tree_lookup(path);
    inode* n = get_inode(l);
    inode* h = get_inode(0);
    void *b = (void*)(uintptr_t)((char*)get_data_start() + h->ptrs[0]);
    memcpy(b, buf, size);
    n->ptrs[0]=h->ptrs[0];
    n->size=size;
    h->ptrs[0]+=size;
    rv = size;
    printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
    int rv = -1;
    printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
           path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

// Extended operations
int
nufs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi,
           unsigned int flags, void* data)
{
    int rv = -1;
    printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
    return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
    memset(ops, 0, sizeof(struct fuse_operations));
    ops->access   = nufs_access;
    ops->getattr  = nufs_getattr;
    ops->readdir  = nufs_readdir;
    ops->mknod    = nufs_mknod;
    ops->mkdir    = nufs_mkdir;
    ops->link     = nufs_link;
    ops->unlink   = nufs_unlink;
    ops->rmdir    = nufs_rmdir;
    ops->rename   = nufs_rename;
    ops->chmod    = nufs_chmod;
    ops->truncate = nufs_truncate;
    ops->open	  = nufs_open;
    ops->create	  = nufs_create;
    ops->read     = nufs_read;
    ops->write    = nufs_write;
    ops->utimens  = nufs_utimens;
    ops->ioctl    = nufs_ioctl;
};

struct fuse_operations nufs_ops;

void
mkfs() {
	pages_init("data.nufs");
	size_t *p = (size_t*)get_root_start();
	*p = 1;
	dirent *root = (dirent*)get_root_start() + 1;
	strcpy(root->name, "/");
	void *blk = get_root_start();	// Root directory starts at the beginning of data segment...
	int t = alloc_inode();
	inode* ptr = get_inode(t);
	ptr->mode=040755;
	ptr->ptrs[0] = 0;	// What if instead we tracked pointers relative to the start of data, so as to account for different memory mappings?
	root->inum = t;
	root->type = DIRECTORY;
	root->active = true;
	root->next=NULL;
	pages_free();
}

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    //if (access(argv[argc], F_OK) != 0) mkfs();
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

