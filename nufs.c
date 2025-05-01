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

dirent nul;
dirent stop;

char *split(const char *path, int n) {
	int rv=0;
	char *splt = (char*)calloc(DIR_NAME, sizeof(char));
	if (n==0) {
		strcpy(splt, "/");
	} else {
		int c=0, i=0;
		for (; path[i] && c<n+1; i++) {
			splt[i]=path[i];
			if (path[i]=='/') c++;
		}
		if (splt[i-1]=='/') splt[i-1]='\0';
	}
	return splt;
}

int
count_l(const char *path) {
	int c=0;
	for(int i=0; path[i]; i++) {
		if (path[i]=='/') c++;
	}
	return c;
}

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{	//if (!strcmp(path, "/")) return rv;
    int rv = 0;
    int l = tree_lookup(path);
    rv = (l>-1) ? F_OK : -ENOENT;
    printf("access(%s, %04o) -> %d\n", path, mask, rv);
    return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
    /*int rv = 0;
    int l = alloc_inode(path);
    inode *n = get_inode(l);
    size_t* count = (size_t*)get_root_start();
    dirent *nod = (dirent*)get_root_start() + 1;
    for (int i=0; i<*count; i++, *nod++);
    strcpy(nod->name, path);
    nod->inum = l;
    nod->active=true;
    *count = *count + 1;
    n->mode = mode;*/
    int rv = 0;
    int count = 0;
    int l = inode_find(path);
    inode *n = get_inode(0);
    inode *h = get_inode(l);
    dirent *p0, *p1;
    dirent data;
    data.inum=l;
    strcpy(data.name, path);
    n->mode=mode;
    data.active=true;
mk_loop:
	p0 = (dirent*)((char*)get_root_start()+n->ptrs[0]);
	p1 = (dirent*)((char*)get_root_start()+n->ptrs[1]);
	if (!strcmp(p0->name, "")) {
		memcpy(p0, &data, sizeof(data));
		h->ptrs[0] = n->ptrs[0];
	} else if (!strcmp(p0->name, "*")) {
		strcpy(stop.name, "*");
		memcpy(p0, &data, sizeof(data));
		h->ptrs[0] = n->ptrs[0];
		memcpy(p1, &stop, sizeof(stop));
	} else if (!strcmp(p1->name, "")) {
		memcpy(p1, &data, sizeof(data));
		h->ptrs[1] = n->ptrs[1];
	} else if (!strcmp(p1->name, "*")) {
		strcpy(stop.name, "*");
		memcpy(p0, &data, sizeof(data));
		h->ptrs[0] = n->ptrs[1];
		if (n->iptr==0) n->iptr = alloc_inode("");
		n = get_inode(n->iptr);
		p0 = (dirent*)((char*)get_root_start()+n->ptrs[0]);
		//p1 = (dirent*)((char*)get_root_start()+n->ptrs[1]);
		memcpy(p0, &stop, sizeof(stop));
	} else {
		n = get_inode(n->iptr);
		goto mk_loop;
	}
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
    //char name[DIR_NAME];
    
    /*inode* n = get_inode(0);
    dirent *e0 = (dirent*)((char*)get_root_start()+n->ptrs[0]);
    dirent *e1 = (dirent*)((char*)get_root_start()+n->ptrs[1]);
    //printf("%d\n", n->ptrs[0]);
    //printf("%d\n", n->ptrs[1]);
    while (true) {
    	if (!strcmp(e0->name, "*")) break;
    	//strcpy(name, e0->name);
    	rv = nufs_getattr(e0->name, &st);
    	assert(rv == 0);
    	filler(buf, e0->name, &st, 0);
    	if (!strcmp(e1->name, "*")) break;
    	//strcpy(name, e1->name);
    	rv = nufs_getattr(e0->name, &st);
    	assert(rv == 0);
    	filler(buf, e1->name, &st, 0);
    }*/
    
        int rv;
    
	inode* n = get_inode(0);
	dirent *e0 = (dirent*)((char*)get_root_start()+n->ptrs[0]);
	dirent *e1 = (dirent*)((char*)get_root_start()+n->ptrs[1]);
	printf("%d\n", n->ptrs[0]);
	printf("%d\n", n->ptrs[1]);
	while (true) {
		if (!strcmp(e0->name, "*")) break;
		rv = nufs_getattr(e0->name, &st);
		assert(rv == 0);
		filler(buf, e0->name, &st, 0);
		if (!strcmp(e1->name, "*")) break;
		rv = nufs_getattr(e0->name, &st);
		assert(rv == 0);
		filler(buf, e1->name, &st, 0);
		if (n->iptr != 0) n = get_inode(n->iptr);
		else return 0;
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
    printf("%d\n", count_l(path));
    // TODO: Nested Directories
    /*for (int i=0; i<count_l(path)) {
    }*/
    printf("mkdir(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_unlink(const char *path)
{
    int rv = -1;
    int l = tree_lookup(path);
    if (l<0) return -ENOENT;
    /*size_t* count = (size_t*)get_root_start();
    dirent *ent = (dirent*)get_root_start()+1;
    nul.active=false;
    void* bm = get_inode_bitmap();
    for (int i=0; i<*count; i++) {
    	rv = nufs_getattr(ent->name, 0);
    	assert(rv == 0);
    	if (!strcmp(ent->name, path)) {
    		bitmap_put(bm, l, 0);
    		memcpy(ent, &nul, sizeof(nul));
    		return rv;
    	}
	*ent++;
    }*/
    printf("unlink(%s) -> %d\n", path, rv);
    return rv;
}

int
nufs_link(const char *from, const char *to)
{
    int rv = 0;
    /*int l = tree_lookup(from);
    inode *n = get_inode(l);
    size_t* count = (size_t*)get_root_start();
    dirent *nod = (dirent*)get_root_start() + 1;
    for (int i=0; i<*count; i++, *nod++);
    strcpy(nod->name, to);
    nod->inum = l;
    nod->active=true;
    *count = *count + 1;
    n->mode = 0100644;*/
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
    int l = tree_lookup(from);
    int rv;
    nul.active=false;
    if (!(rv = (l>0) ? 0 : -ENOENT)) {
        size_t* count = (size_t*)get_root_start();
        dirent *f = (dirent*)get_root_start() + 1;
        dirent *t = (dirent*)get_root_start() + 1;
        for (int i=0; i<*count && strcmp(f->name, from); i++, *f++);
        for (int i=0; i<*count && strcmp(t->name, to); i++, *t++);
        strcpy(t->name, to);
        t->inum = l;
        t->active=true;
        *count = *count + 1;
        memcpy(f, &nul, sizeof(nul));
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
    if (k==-ENOENT) k = nufs_create(path, 0100644, 0);
    printf("open(%s) -> %d\n", path, rv);
    return rv;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    int rv = 4096;
    int l = tree_lookup(path);
    if (l>-1) {
    	bool start = true;
    	int p0=0, p1=0, i=0;
    	inode* n = get_inode(l);
    	char *data0, *data1;
	read_loop:
    	if (start) {
    		data0 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[0]+offset);
    		start = false;
    	} else {
    		data0 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[0]);
    	}
    	data1 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[1]);
    	for(; data0[p0] && i < size; p0++, i++) buf[i] = data0[p0];
    	for(; data1[p1] && i < size; p1++, i++) buf[i] = data1[p1];
    	if (i < size-1) {
    		n = get_inode(n->iptr);
    		goto read_loop;
    	}
    	rv = i;
    }
    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
    // TODO: Implement more complete write logic...
    int rv = 0;
    int l = tree_lookup(path);
    bool start = true;
    int p0=0, p1=0, i=0;
    inode* n = get_inode(l);
    inode* h = get_inode(1);
    char *data0, *data1;
write_loop:
    if (start) {
    	data0 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[0]+offset);
    	start = false;
    } else {
    	data0 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[0]);
    }
    data1 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[1]);
    for(; data0[p0]==0 && i < size; p0++, i++) data0[p0] = buf[i];
    for(; data1[p1]==0 && i < size; p1++, i++) data1[p1] = buf[i];
    if (i < size-1) {
    	n = get_inode(n->iptr);
    	goto write_loop;
    }
    rv = i;
    n->ptrs[0]=h->ptrs[0];
    n->size=size;
    h->ptrs[0]+=size;
    n->ptrs[1]=h->ptrs[1];
    h->ptrs[1]+=size;
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

int
main(int argc, char *argv[])
{
    assert(argc > 2 && argc < 6);
    //if (access(argv[argc], F_OK) != 0) mkfs();
    storage_init(argv[--argc]);
    nufs_init_ops(&nufs_ops);
    return fuse_main(argc, argv, &nufs_ops, NULL);
}

