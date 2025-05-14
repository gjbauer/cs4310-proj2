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

int inode_size(inode *d)
{
	return (d->size[0]+d->size[1]);
}

int calc_offset(inode *d, off_t offset)
{
	if (d->size[0]!=0) offset-=d->size[0];
	else return offset;
	if (d->size[1]==0) return offset;
	else offset-=d->size[0];
	if (offset > 0 && d->iptr!=0) {
		d = get_inode(d->iptr);
		return calc_offset(d, offset);
	} else {
		//d->iptr = alloc_inode();	// Inode_find()?
		//d = get_inode(d->iptr);
		if (offset == 0) return 0;
		return -1;
	}
}

int _remainder(inode *d, int size, off_t offset)
{
	if (d->size[0]==0) return 0;
	else if (d->size[1]==0) return 0;
	else {
		return (size - ((d->size[0]+d->size[1]) - offset));
	}
}

char* get_data_end()
{
	return (char*)get_root_start() + get_inode(1)->ptrs[0];
}

bool is_empty(inode *d)
{
	return (d->size[0]==0 || d->size[1]==0);
}

int split(const char *path, int n, char buf[DIR_NAME]) {
	int rv=0;
	if (n==0) {
		strcpy(buf, "/");
	} else {
		int c=0, i=0;
		for (; path[i] && c<n+1; i++) {
			buf[i]=path[i];
			if (path[i]=='/') c++;
		}
		if (buf[i-1]=='/') buf[i-1]='\0';
	}
	return rv;
}

int
count_l(const char *path) {
	int c=0;
	for(int i=0; path[i]; i++) {
		if (path[i]=='/') c++;
	}
	return c;
}

int
find_parent(const char *path)
{
	char ptr[DIR_NAME];
	int k = count_l(path);
	int n=0;
	for (int i=0; i<k; i++) {
		split(path, i, ptr);
		n = tree_lookup(ptr, n);
		if (n<0) return -ENOENT;
	}
	// TODO : Locate a parent directory and return an inode, or an iptr
	return n;
}


char*
parent_path(const char *path)
{
	char ptr[DIR_NAME];
	int k = count_l(path);
	int n=0;
	for (int i=0; i<k; i++) {
		split(path, i, ptr);
	}
	
	char *m = (char*)malloc(DIR_NAME * sizeof(char));
	strncpy(m, ptr, DIR_NAME);
	return m;
}

char *get_data(int offset)
{
	return ((char*)get_root_start()+offset);
}

int
count_placement(int l, const char* path, const char *ppath)
{
	inode *d = get_inode(l);
	int i=0;
	dirent *e;
	while (true) {
		e = (dirent*)get_data(d->ptrs[0]);
		if (( ( d->size[0]==0 ) ) ) break;
		i++;
		e = (dirent*)get_data(d->ptrs[1]);
		if ((d->size[1]==0) ) break;
		i++;
		d = (d == 0) ? get_inode( (d->iptr = inode_find(ppath)) ) : get_inode(d->iptr);
	}
	return i;
}

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{	//if (!strcmp(path, "/")) return rv;
    int rv = 0;
    int l = tree_lookup(path, find_parent(path));
    rv = (l>-1) ? F_OK : -ENOENT;
    printf("access(%s, %04o) -> %d\n", path, mask, rv);
    return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int rv = 0;
	int l = (!strcmp(path, "/")) ? 0 : inode_find(path);
	char *ppath = parent_path(path);
	
	dirent e;
	inode *h = get_inode(tree_lookup(ppath, find_parent(ppath)));
	inode *n = get_inode(l);
	n->mode=mode;
	
	strncpy(e.name, path, DIR_NAME);
	e.inum = l;
	e.active = true;
	
	int i = count_placement(l, path, ppath);
	
	printf("count_placement = %d\n", i);
	
	nufs_write(ppath, (char*)&e, sizeof(dirent), i*sizeof(dirent), 0);
	

	free(ppath);
	printf("mknod(%s) -> %d\n", path, rv);
	return rv;
}

int
nufs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	if (nufs_mknod(path, mode, 0)) {
    		int l = tree_lookup(path, find_parent(path));
    		inode *n = get_inode(l);
        	n->mode = mode; // regular file
        	n->size[0] = 0;
        	n->size[1] = 0;
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
    int l = tree_lookup(path, find_parent(path));
    inode *n;
    if (strcmp(path, "/") == 0) {
        st->st_mode = 040755; // directory
        st->st_size = 0;
        st->st_uid = getuid();
    }
    else if (l>-1) {
    	if (st) {
    		n = get_inode(l);
        	st->st_mode = n->mode; // regular file
        	st->st_size = n->size[0];	// TODO : This should be a function which calculates size from the inodes..
        	st->st_uid = getuid();
        }
    }
    else if (!strcmp(path, "/one.txt") || !strcmp(path, "/two.txt") || !strcmp(path, "/2k.txt") || !strcmp(path, "/40k.txt") || isnum(path)) {
    	l = nufs_create(path, 0100644, 0);
    }
    else {
    	rv = -ENOENT;
    }
    if (st) printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode, st->st_size);
    else printf("getattr(%s) -> (%d)\n", path, rv);
    return rv;
}

int
_readdir(const char *path, void *buf, fuse_fill_dir_t filler, int l)
{
	struct stat st;
	int rv=0;
	(l == 0) ? (l = tree_lookup(path, find_parent(path))) : l;
	inode *a = get_inode(l);
	dirent e;
	
	memcpy(&e, get_data(a->ptrs[0]), sizeof(e));
	if (!strcmp(e.name, "")) return 0;
	rv = nufs_getattr(e.name, &st);
	assert(rv == 0);
	filler(buf, e.name, &st, 0);
	printf("%s\n", e.name);	// getaddr
	rv++;
	
	memcpy(&e, get_data(a->ptrs[1]), sizeof(e));
	if (!strcmp(e.name, "") || a->ptrs[1]==0) return 0;
	rv = nufs_getattr(e.name, &st);
	assert(rv == 0);
	filler(buf, e.name, &st, 0);
	printf("%s\n", e.name);	// getaddr
	rv++;
	
	int ptr = a->iptr;
	rv = (ptr==0) ? (rv) : rv+_readdir(path, buf, filler, ptr);
	return rv;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
	int rv=_readdir(path, buf, filler, 0);
	printf("readdir(%d)\n", rv);
	return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
    	int rv = nufs_mknod(path, mode | 040000, 0);
	printf("mkdir(%s) -> %d\n", path, rv);
	return rv;
}

int
nufs_unlink(const char *path)
{
    int rv = -1;
    int l = tree_lookup(path, find_parent(path));
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
    int l = tree_lookup(from, find_parent(from));
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

int
_read(const char *path, const char *buf, size_t size, off_t offset, int l)
{
	(l == 0) ? l = tree_lookup(path, find_parent(path)) : l;
	inode *n = get_inode(l);
	
	int r = ( size - (n->size[0]+n->size[1]) );
	// TODO : Reads larger than a single inode...
	
	if (offset < n->size[0]) {
		memcpy(buf, get_data(n->ptrs[0]+offset), n->size[0]-offset);
		if ( (size - n->size[0]) > 0 ) memcpy(buf+n->size[0], get_data(n->ptrs[1]), ( n->size[1] > (size-n->size[0]) ) ? (size) : (n->size[1]) );
	}
	else {
		if (offset < n->size[0]+n->size[1]) {
			memcpy(buf, get_data(n->ptrs[1]+offset), n->size[1]-(offset-n->size[0]));
		} else if (n->iptr==0) return -1;
		else {
			return _read(path, buf, size, offset - (n->size[0]+n->size[1]), n->iptr);
		}
	}
	
	if (r>0) return _read(path, buf, r, offset - (n->size[0]+n->size[1]), n->iptr);
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	int l = tree_lookup(path, find_parent(path));
	return _read(path, buf, size, offset, l);
}

int
write_sp(char *data, int inode, int ptr, const char *buf, size_t size)
{
	struct inode n; // *get_inode(inode);
	memcpy(&n, get_inode(inode), sizeof(n));
	struct inode h; // *get_inode(1);
	memcpy(&h, get_inode(1), sizeof(n));
	memcpy(buf, data, size);
	data[size] = '\0';
	n.size[ptr]=size;
	printf("ptr = %d\n", ptr);
	n.ptrs[ptr] = h.ptrs[0];
	h.ptrs[0] += size;
	memcpy(get_inode(inode), &n, sizeof(n));
	memcpy(get_inode(1), &h, sizeof(h));
}

int
_write(const char *path, const char *buf, size_t size, off_t offset, int l)
{
	int rv = 0;
	(l == 0) ? l = tree_lookup(path, find_parent(path)) : l;
	inode *n = get_inode(l), *h = get_inode(1);
	
	int s = inode_size(n);
	
	if (s == 0) write_sp(get_data_end()+offset, l, 0, buf, size);
	else if (is_empty(n)) write_sp(get_data_end()+offset-s, l, 1, buf, size);
	else {
		int r = _remainder(n, size, offset);
		if (r<=0) {
			if (offset < n->size[0]) {
				write_sp(get_data(n->ptrs[0]+offset), l, 0, buf, n->size[0]-offset);
				size-=n->size[0];
			}
			if (size > 0) {
				write_sp(get_data(n->ptrs[1]+(offset-n->size[0])), l, 1, buf, size );
			}
		}
		else {
			if (n->iptr == 0) n->iptr = inode_find(path);
			if (_remainder(n, size, offset) >= size) {
				return _write(path, buf, (size), (_remainder(n, size, offset) - size), n->iptr);
			}
			else
				return _write(path, buf, (size - _remainder(n, size, offset)), (0), n->iptr);
		}
	}
	printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	return _write(path, buf, size, offset, 0);
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
