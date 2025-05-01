#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"

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
    	n = get_inode(n->iptr);
    }
}

int
main(int argc, char *argv[])
{
	storage_init("data.nufs");
	readdir();
	pages_free();
}
