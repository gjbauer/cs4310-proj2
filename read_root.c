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
    inode *n = get_inode(1);
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

int
main(int argc, char *argv[])
{
	storage_init("data.nufs");
	readdir();
	mknod("/hello.txt");
	readdir();
	pages_free();
}
