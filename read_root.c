#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"

dirent stop;

int readdir()
{
    int rv;
    
    inode* n = get_inode(0);
    dirent *e0 = (dirent*)((char*)get_root_start()+n->ptrs[0]);
    dirent *e1 = (dirent*)((char*)get_root_start()+n->ptrs[1]);
    printf("%d\n", n->ptrs[0]);
    printf("%d\n", n->ptrs[1]);
    while (true) {
    	if (!strcmp(e0->name, "*")) break;
    	rv = printf("%s\n", e0->name);
    	if (!strcmp(e1->name, "*")) break;
    	rv = printf("%s\n", e0->name);
    	if (n->iptr != 0) n = get_inode(n->iptr);
    	else return 0;
    }
}

int
mknod(const char *path)
{
    int rv = 0;
    int count = 0;
    int l = alloc_inode(path);
    inode *n = get_inode(0);
    inode *h = get_inode(l);
    dirent *p0, *p1;
    dirent data;
    data.inum=l;
    strcpy(data.name, path);
    //n->mode=mode;
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
    printf("mknod(%s) -> %d\n", path, rv);
    return rv;
}

// Actually write data
int
write(const char *path, const char *buf, size_t size, off_t offset)
{
    int rv = 0;
    int l = tree_lookup(path);
    bool start = true;
    int p0=0, p1=0, i=0;
    inode* n = get_inode(l);
    inode* h = get_inode(1);
    printf("buf : %s\n", buf);
    char *data0, *data1;
    int i0, i1;
write_loop:
    if (start) {
    	data0 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[0]+offset);
    	start = false;
    } else {
    	data0 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[0]);
    }
    data1 = (void*)(uintptr_t)((char*)get_data_start()+n->ptrs[1]);
    for(; data0[p0]==0 && i < size; p0++, i++);
    strncpy(data0, buf, p0);
    for(; data1[p1]==0 && i < size; p1++, i++);
    strncpy(data1, (buf+p0), p0);
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

// Actually read data
int
read(const char *path, char *buf, size_t size, off_t offset)
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
    	for(; data0[p0] && i < size; p0++, i++);
    	strncpy(buf, data0, p0);
    	for(; data1[p1] && i < size; p1++, i++);
    	strncpy(buf, data1, p1);
    	if (i < size-1) {
    		n = get_inode(n->iptr);
    		goto read_loop;
    	}
    	rv = i;
    }
    printf("buf : %s\n", buf);
    printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
    return rv;
}

int
main(int argc, char *argv[])
{
	char buf[256];
	storage_init("data.nufs");
	readdir();
	mknod("/hello.txt");
	readdir();
	write("/hello.txt", "hello!", 6, 0);
	read("/hello.txt", buf, 0, 0);
	printf("%s\n", buf);
	pages_free();
}
