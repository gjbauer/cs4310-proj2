#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mkfs.h"

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
	int l = find_parent(path);
	l = tree_lookup(path, l);
	
	inode* n = get_inode(l);
	dirent *e0;
	dirent *e1;
	
	while (true) {
		e0 = (dirent*)((char*)get_root_start()+n->ptrs[0]);
		e1 = (dirent*)((char*)get_root_start()+n->ptrs[1]);
		//printf("%s\n", e0->name);	// getaddr
		if (!strcmp(e0->name, "*")) break;
		printf("%s\n", e0->name);	// getaddr
		//printf("%s\n", e1->name);
		if (!strcmp(e1->name, "*")) break;
		printf("%s\n", e1->name);
		printf("readdir: getting next inode!\n");
		n = get_inode(n->iptr);
	}
	printf("readdir(%d)\n", rv);
	return rv;
}

int
_mknod(const char *path, int mode, int k)
{
	int rv = 0;
	int count = 0;
	int l = inode_find(path);
	inode *n = get_inode(1);
	printf("find_parent(path) = %d\n", find_parent(path));
	inode *p = get_inode(k);	// <- parent directory
	inode *h = get_inode(l);
	h->refs=1;
	dirent *p0, *p1, *w;
	dirent data;
	data.inum=l;
	strcpy(data.name, path);
	h->mode=mode;
	data.active=true;
	p0 = (dirent*)((char*)get_root_start()+p->ptrs[0]);
	p1 = (dirent*)((char*)get_root_start()+p->ptrs[1]);
	w = (dirent*)((char*)get_root_start()+n->ptrs[0]);
	if (!strcmp(p0->name, "*")) {
		printf("p0: found stop!\n");
		strcpy(stop.name, "*");
		memcpy(p0, &data, sizeof(data));
		p->ptrs[1] = n->ptrs[0];
		memcpy(w, &stop, sizeof(stop));
		n->ptrs[0] += sizeof(stop);
		n->ptrs[1] += sizeof(stop);	
	} /*else if (p0->active==false) {
		printf("p0: found empty!\n");
		memcpy(p0, &data, sizeof(data));
	} else if (p1->active==false) {
		printf("p1: found empty!\n");
		memcpy(p1, &data, sizeof(data));
	}*/ else if (!strcmp(p1->name, "*")) {
		printf("p1: found stop!\n");
		memcpy(p1, &data, sizeof(data));
		p->iptr = inode_find("*");
		get_inode(p->iptr)->ptrs[0] = n->ptrs[0];
		n->ptrs[0] += sizeof(data);
		n->ptrs[1] += sizeof(data);
		memcpy(w, &stop, sizeof(stop));
	} else {
		printf("found none, getting next inode!\n");
		_mknod(path, mode, p->iptr);
	}
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
	return ((040000 ^ mode) < 010000) ? true : false;
}

// Actually write data
int
write(const char *path, const char *buf, size_t size, off_t offset)
{
	int rv = 0;
	int l = tree_lookup(path, find_parent(path));
	bool start = true;
	int p0=0, p1=0, i=0;
	inode* n = get_inode(l);
	inode* h = get_inode(1);
	char *data0, *data1;
	int i0, i1;
	//printf("h->ptrs[0] = %d\n", h->ptrs[0]);	// TODO : Move seperately after file unlink
	//printf("h->ptrs[1] = %d\n", h->ptrs[1]);
	if (start) {
		data0 = ((char*)get_root_start()+h->ptrs[0]+offset);
		start = false;
	} else {
		data0 = ((char*)get_root_start()+h->ptrs[0]);
	}
	if (offset > n->size[0]) {
		offset -= n->size[0];
		data1 = ((char*)get_root_start()+h->ptrs[1]+offset);
		for(; i < size; p0++, i++) data0[p0] = buf[i];
		n->size[0]=p0;
		n->ptrs[0] = h->ptrs[0];
		h->ptrs[0] += size;
		h->ptrs[1] += size;
	} else {
		data1 = ((char*)get_root_start()+h->ptrs[1]);
		//data0[0]='\0';	//TODO : Worry about write collision later...
		if (n->size[0] > 0) {
			strncpy(data0, buf, n->size[0]);
			strncat(data1 + (int)n->size[0], buf+n->size[0], n->size[1]);
			data1[n->size[0] + n->size[1]] = '\0';
			n->size[1]=p1;
			n->ptrs[0] = h->ptrs[0];
			h->ptrs[0] += n->size[0];
			n->ptrs[1] = h->ptrs[1];
			h->ptrs[1] += n->size[1];
		} else {
			strncpy(data0, buf, size);
			data0[size] = '\0';
			n->size[0]=size;
			n->ptrs[0] = h->ptrs[0];
			h->ptrs[0] += size;
			h->ptrs[1] += size;
		}
	}
	printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}

// Actually read data
int
read(const char *path, char *buf, size_t size, off_t offset)
{
	int rv = 4096;
	int l = tree_lookup(path, find_parent(path));
	//printf("l = %d\n", l);
	bool start = true;
	int p0=0, p1=0, i=0;
	inode* n = get_inode(l);
	char *data0, *data1;
	if (l>-1) {
//read_loop:
		if (start) {
			data0 = ((char*)get_root_start()+n->ptrs[0]+offset);
			start = false;
		} else {
			data0 = ((char*)get_root_start()+n->ptrs[0]);
		}
		data1 = ((char*)get_root_start()+n->ptrs[1]);
		// TODO : Bounds checking
		strncpy(buf, data0, n->size[0]);
		strncat(buf, data1, n->size[1]);
		/*if (n->size[0] + n->size[1] < size) {
			n = get_inode(n->iptr);
			goto read_loop;
		}*/
	}
	printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
mkdir(const char *path, int mode)
{
	int rv = mknod(path, mode | 040000);
	//printf("%d\n", count_l(path));
	strcpy(stop.name, "*");
	write(path, (char*)&stop, sizeof(stop), 0);
	// TODO: Nested Directories
	/*for (int i=0; i<count_l(path)) {
	}*/
	printf("mkdir(%s) -> %d\n", path, rv);
	return rv;
}

int
main(int argc, char *argv[])
{
	char buf[256];
	storage_init("data.nufs");
	readdir("/");	// Empty
	mknod("/hello.txt", 755);
	readdir("/");
	mkdir("/dir", 755);
	write("/hello.txt", "hello!", 6, 0);
	readdir("/");
	read("/hello.txt", buf, 0, 0);
	printf("%s\n", buf);	// hello!
	mknod("/dir/newmsg.txt", 755);
	write("/dir/newmsg.txt", "newmsg!", 6, 0);
	read("/dir/newmsg.txt", buf, 0, 0);
	printf("reading root\n");
	readdir("/");
	printf("reading /dir\n");
	readdir("/dir");
	read("/hello.txt", buf, 0, 0);
	printf("%s\n", buf);	// hello!
	//printf("%s\n", buf);	// newmsg!
	mknod("/dir/two.txt", 755);
	//write("/dir/two.txt", "two!", 6, 0);
	//read("/dir/two.txt", buf, 0, 0);
	//printf("%s\n", buf);	// two!
	readdir("/dir");
	mkdir("/dir/dir", 755);
	mknod("/dir/dir/one.txt", 755);
	printf("reading /dir/dir\n");
	readdir("/dir/dir");
	write("/dir/dir/one.txt", "one!", 6, 0);
	printf("reading /dir/dir/one.txt\n");
	read("/dir/dir/one.txt", buf, 0, 0);
	printf("%s\n", buf);
	printf("reading /dir\n");
	readdir("/dir");
	//read("/hello.txt", buf, 0, 0);
	//printf("%s\n", buf);	// hello!
	pages_free();
}
