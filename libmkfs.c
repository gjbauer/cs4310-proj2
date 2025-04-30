#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mkfs.h"
#include <stdlib.h>

void
mkfs() {
	pages_init("data.nufs");
	size_t *p = (size_t*)get_root_start();
	*p = 1;
	dirent *root = (dirent*)get_root_start() + 1;
	strcpy(root->name, "/");
	void *blk = get_root_start();	// Root directory starts at the beginning of data segment...
	int t = alloc_inode("/");
	inode* ptr = get_inode(t);
	ptr->mode=040755;
	ptr->ptrs[0] = 0;	// What if instead we tracked pointers relative to the start of data, so as to account for different memory mappings?
	root->inum = t;
	root->type = DIRECTORY;
	root->active = true;
	root->next=NULL;
	pages_free();
}

