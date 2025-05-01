#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"

int readdir()
{
    int rv;
    //char name[DIR_NAME];
    
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
    	n = get_inode(n->iptr);
    }
}

// Actually read data
int
read(const char *path, char *buf, size_t size)
{
    int rv = 6;
    //strcpy(buf, "hello\n");
    int l = tree_lookup(path);
    inode* n = get_inode(l);
    void *data = (void*)(uintptr_t)n->ptrs[0];
    memcpy(buf, get_data_start(), size);
    printf("read(%s, %ld bytes)\n", path, size);
    return rv;
}

// Actually write data
int
write(const char *path, const char *buf, size_t size)
{
    int rv = -1;
    int l = tree_lookup(path);
    if (l==-1) {
    	int l = alloc_inode(path);
    }
    inode* n = get_inode(l);
    //void *b = (void*)(uintptr_t)n->ptrs[0];
    memcpy(get_data_start(), buf, size);
    printf("write(%s, %ld bytes)\n", path, size);
    return rv;
}

int
main(int argc, char *argv[])
{
	storage_init("data.nufs");
	//dirent *root = (dirent*)get_root_start();	// Root directory starts at the beginning of data segment...
	readdir();
	pages_free();
}
