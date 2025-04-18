#include "pages.h"
#include "bitmap.h"
#include "inode.h"
#include <stdint.h>
//void print_inode(inode* node) {}
inode* get_inode(int inum) {
	// TODO: Return a specific inode given a number...
	void *ptr = get_inode_start();
	return (void*)((inode*)ptr + inum);
}
int alloc_inode() {
	// TODO: Create an inode, mark it off on the bitmap, return its number...
	void* ibm = get_inode_bitmap();
	for (int i=0; i<512; i++) {
		if (!bitmap_get(ibm, i)) {
			bitmap_put(ibm, i, 1);
			return i;
		}
	}
	return -1;
}
//void free_inode() {}
//int grow_inode(inode* node, int size) {}
//int shrink_inode(inode* node, int size) {}
//int inode_get_pnum(inode* node, int fpn) {}
