#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"

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
	dirent *root = (dirent*)get_root_start()+1;	// Root directory starts at the beginning of data segment...
	size_t* count = (size_t*)get_root_start();
	int i=0;
	while (i<*count) {
		//printf("%s\n", root->name);
		//printf("%d\n", root->inum);
		*root++, i++;
	}
	char buff[256];
	//write("/hello.txt", "hello\n", 6);
	read("/hello.txt", buff, 6);
	printf("%s\n", buff);
	pages_free();
}
