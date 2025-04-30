#include "pages.h"
#include "bitmap.h"
#include "inode.h"
#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
//void print_inode(inode* node) {}
inode* get_inode(int inum) {
	// TODO: Return a specific inode given a number...
	void *ptr = get_inode_start();
	return (void*)((inode*)ptr + inum);
}

int
alloc_inode(const char *path) {
	char *hpath;
	void* ibm = get_inode_bitmap();
    if (!strcmp(path, "/")) {
        bitmap_put(ibm, 0, 1);
        return 0;
    }
	if (bitmap_get(ibm, hash(path))) {
		return alloc_inode(extend(path));
	} else {
		bitmap_put(ibm, hash(path), 1);
		return hash(path);
	}
}
//void free_inode() {}
//int grow_inode(inode* node, int size) {}
//int shrink_inode(inode* node, int size) {}
//int inode_get_pnum(inode* node, int fpn) {}
