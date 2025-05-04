#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mkfs.h"
#include "mfs.h"

dirent stop;

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
	char trm[DIR_NAME];
	int i;
	for(i=0; path[i]; i++) trm[i]=path[i];
	if (trm[i]=='/') trm[i]='\0';
	char ptr[DIR_NAME];
	int k = count_l(trm);
	int n=0;
	for (int i=0; i<k; i++) {
		split(trm, i, ptr);
		n = tree_lookup(ptr, n);
		if (n<0) return -ENOENT;
	}
	// TODO : Locate a parent directory and return an inode, or an iptr
	return n;
}

int
readdir(const char *path)
{
	int rv=0;
	int l = (!strcmp(path, "/")) ? tree_lookup(path, find_parent(path)) : 0;
	
	dirent e;
	
	inode* n = get_inode(l);
	
	while (true) {
		read(path, (char*)&e, sizeof(e), 0);
		printf("%s\n", e.name);	// getaddr
		if (!strcmp(e.name, "*")) break;
		//printf("%s\n", e0->name);	// getaddr
		printf("%s\n", e.name);
		read(path, (char*)&e, sizeof(e), sizeof(e));
		if (!strcmp(e.name, "*")) break;
		//printf("%s\n", e1->name);
		printf("readdir: getting next inode!\n");
		break;
		//n = get_inode(n->iptr);
	}
	printf("readdir(%d)\n", rv);
	return rv;
}

int
_mknod(const char *path, int mode, int k)
{
	int rv = 0;
	int l = (!strcmp(path, "/")) ? 0 : inode_find(path);
	inode *h = get_inode(l);
	inode h0;
	h0.refs=1;
	h0.mode=mode;
	memcpy(h, &h0, sizeof(h0));
	dirent data;
	data.inum=l;
	strncpy(data.name, path, DIR_NAME);
	strncpy(stop.name, "*", DIR_NAME);
	data.active=true;
	write(path, (char*)&data, sizeof(data), 0);
	write(path, (char*)&stop, sizeof(stop), sizeof(data));
	printf("mknod(%s) -> %d\n", path, rv);
	return rv;
}

int
mknod(const char *path, int mode)
{
	return _mknod(path, mode, find_parent(path));
}

int
is_dir(int mode)
{
	return (mode > 040000) ? true : false;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
mkdir(const char *path, int mode)
{
	int rv = mknod(path, mode | 040000);
	//printf("%d\n", count_l(path));
	// TODO: Nested Directories
	/*for (int i=0; i<count_l(path)) {
	}*/
	printf("mkdir(%s) -> %d\n", path, rv);
	return rv;
}

int
write_sp(char *data, int inode, int ptr, const char *buf, size_t size)
{
	struct inode n; // *get_inode(inode);
	memcpy(&n, get_inode(inode), sizeof(n));
	struct inode h; // *get_inode(1);
	memcpy(&h, get_inode(1), sizeof(n));
	memcpy(data, buf, size);
	data[size] = '\0';
	n.size[ptr]=size;
	n.ptrs[ptr] = h.ptrs[0];
	h.ptrs[0] += size;
	h.ptrs[1] += (h.ptrs[0]==h.ptrs[1]) ? size : 0;
	memcpy(get_inode(inode), &n, sizeof(n));
	memcpy(get_inode(1), &h, sizeof(h));
}

int
write_mp(char *data0, char *data1, int inode, const char *buf, size_t size)
{
	struct inode n; // *get_inode(inode);
	memcpy(&n, get_inode(inode), sizeof(n));
	struct inode h; // *get_inode(1);
	memcpy(&h, get_inode(1), sizeof(n));
	memcpy(data0, buf, n.size[0]);
	memcpy(data1 + (int)n.size[0], buf+n.size[0], n.size[1]);
	data1[n.size[0] + n.size[1]] = '\0';
	//n->size[1]=;//TODO : second inode size
	h.ptrs[0] += n.size[0];
	n.ptrs[1] = h.ptrs[1];
	h.ptrs[1] += n.size[1];
	memcpy(get_inode(inode), &n, sizeof(n));
	memcpy(get_inode(1), &h, sizeof(h));
}

// Actually write data
int
_write(const char *path, const char *buf, size_t size, off_t offset, int l)
{
	int rv = 0;
	(l == 0) ? l = tree_lookup(path, find_parent(path)) : l;
	
	inode *n = get_inode(l), *h = get_inode(1);
	char *data0, *data1;
	int r=0;
	
	if (offset > (n->size[0] + n->size[1]) && (n->size[0] + n->size[1]) > 0) _write(path, buf+(size - r), size, offset-(n->size[0] + n->size[1]), (n->iptr==0) ? (n->iptr = inode_find(path)) : (n->iptr = n->iptr));
	
	data0 = ((char*)get_root_start()+h->ptrs[0]+offset);
	data1 = (offset >= n->size[0]) ? ((char*)get_root_start()+h->ptrs[1] + (offset - n->size[0])) : ((char*)get_root_start()+h->ptrs[1]);
	
	r = (size > (n->size[0] + n->size[1]) && (n->size[0] + n->size[1]) > 0) ? (size - (n->size[0] + n->size[1])) : 0 ;
	
	if (offset >= n->size[0] && offset != 0) {
		printf("write_sp(data1)\n");
		write_sp(data1, l, 1, buf, size);
	} else {
		if (n->size[0] > 0) {
			printf("write_mp()\n");
			write_mp(data0, data1, l, buf, size-r);
		} else {
			printf("write_sp(data0)\n");
			write_sp(data0, l, 0, buf, size);
		}
	}
	
	if (r > 0) _write(path, buf+(size - r), r, offset, (n->iptr==0) ? (n->iptr = inode_find(path)) : (n->iptr = n->iptr));
	printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}

int
write(const char *path, const char *buf, size_t size, off_t offset)
{
	return _write(path, buf, size, offset, 0);
}

// Actually read data
int
_read(const char *path, char *buf, size_t size, off_t offset, int l)
{
	if (size==0) return -1;
	if (l<0) return -ENOENT;
	
	int rv = 4096;
	inode* n = get_inode(l);
	
	char *data0 = ((char*)get_root_start()+n->ptrs[0]+offset), *data1 = ((char*)get_root_start()+n->ptrs[1]);
	if (n->size[0] <= size) {
		strncpy(buf, data0, n->size[0]);
		strncat(buf, data1, n->size[1]);
		buf[n->size[0]+n->size[1]]='\0';
	} else {
		strncpy(buf, data0, size);
		buf[size]='\0';
	}
	
	if ((n->size[0]+n->size[1])-offset < size) _read(path, buf, size, (size-(n->size[0]+n->size[1])-offset), n->iptr);
	printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}

int
read(const char *path, char *buf, size_t size, off_t offset)
{
	int l = tree_lookup(path, find_parent(path));
	return _read(path, buf, size, offset, l);
}
