#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mkfs.h"
#include <stdlib.h>


int mkstop() {
    int rv = 0;
    int count = 0;
    //int l = alloc_inode("*");	// Why waste an inode?
    inode *n = get_inode(0);
    inode *h = get_inode(1);
    dirent *p;
    dirent data;
    data.inum=0;
    strcpy(data.name, "*");
    n->mode=0;
    data.active=true;
    p = (dirent*)((char*)get_root_start()+n->ptrs[0]);
    memcpy(p, &data, sizeof(data));
    h->ptrs[0]+=sizeof(data);
    h->ptrs[1]+=sizeof(data);
    printf("mkstop(%s) -> %d\n", "*", rv);
    return rv;
}

void
mkfs() {
	pages_init("data.nufs");
	dirent *root = (dirent*)get_root_start();
	strcpy(root->name, "/");
	void *blk = get_root_start();	// Root directory starts at the beginning of data segment...
	inode* ptr = get_inode(0);
	ptr->mode=040755;
	ptr->ptrs[0] = sizeof(root);	// What if instead we tracked pointers relative to the start of data, so as to account for different memory mappings?
	ptr->ptrs[1] = sizeof(root);
	root->inum = 0;
	root->type = DIRECTORY;
	root->active = true;
	root->next=NULL;
	ptr = get_inode(1);
	ptr->ptrs[0] = sizeof(root);	// What if instead we tracked pointers relative to the start of data, so as to account for different memory mappings?
	ptr->ptrs[1] = sizeof(root);	// We can set this when we unlink/remove a file
	mkstop();
	ptr = get_inode(0);
	pages_free();
}

