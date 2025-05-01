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
	int* ptr = (int*)get_inode_bitmap();
	for (int i=0; i<512; i++) {
		if (*ptr==0) {
			if (get_inode(i)->size>0) return i;
		}
	}
	return alloc_inode(path);
}

int
alloc_inode(const char *path) {
	char *hpath;
	char tpath[DIR_NAME];
	void* ibm = get_inode_bitmap();
    if (!strcmp(path, "/")) {
        bitmap_put(ibm, 0, 1);
        return 0;
    }
	if (bitmap_get(ibm, hash(path))==1) {
		//printf("bitmap : %d\n", bitmap_get(ibm, hash(path)));
		hpath = extend(path);
		//printf("extend : %s\n", hpath);
		strcpy(tpath, hpath);
		free(hpath);
		//return alloc_inode(tpath);
		return 0;
	} else {
		bitmap_put(ibm, hash(path), 1);
		return hash(path);
	}
}
//void free_inode() {}
//int grow_inode(inode* node, int size) {}
//int shrink_inode(inode* node, int size) {}
//int inode_get_pnum(inode* node, int fpn) {}
