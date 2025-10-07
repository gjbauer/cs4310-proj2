#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "string.h"
#include <stdlib.h>

void
mkfs() {
	pages_init("data.nufs");
	dirent root;
	strcpy(root.name, "/");
	root.inum = 0;
	root.next=false;
	inode root_node;
	root_node.ptrs[0]=alloc_page();
	root_node.size=sizeof(dirent);
	memcpy(get_root_start(), (char*)&root, sizeof(dirent));
	memcpy(get_inode(alloc_inode()), (char*)&root_node, sizeof(inode));
	memcpy((char*)&root_node, get_inode(0), sizeof(inode));
	printf("root_node.ptrs[0] = %d\n", root_node.ptrs[0]);
	//write("/", (char*)&d, sizeof(d), 0);
	//readdir("/");
	//mknod("/two.txt", 755);
	//readdir("/");
	pages_free();
}

