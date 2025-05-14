#include "pages.h"
#include "bitmap.h"
#include "inode.h"
#include "hash.h"
#include "directory.h"
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
inode_find(const char *path) {
	void* ptr = (void*)get_inode_bitmap();
	for (int i=2; i<512; i++) {
		if (bitmap_get(ptr, i)==0) {
			if (get_inode(i)->size[0]>0&&get_inode(i)->refs==0) return i;
		}
	}
	return alloc_inode(path);
}

int
alloc_inode(const char *path) {
	char *hpath;
	static char str[DIR_NAME];
	void* ibm = get_inode_bitmap();
	if (!strcmp(path, "/")) {
		bitmap_put(ibm, 0, 1);
		return 0;
	} /*else if (bitmap_get(ibm, hash(path))==1) {	// TODO: Fix this! We need the feature, but it is overflowing the stack...
		strncpy(str, path, DIR_NAME);
		str[strlen(path)-1] = hash(path);
		return alloc_inode(str);
	}*/ else {
		bitmap_put(ibm, hash(path), 1);
		return hash(path);
	}
}
//void free_inode() {}
//int grow_inode(inode* node, int size) {}
//int shrink_inode(inode* node, int size) {}
//int inode_get_pnum(inode* node, int fpn) {}
