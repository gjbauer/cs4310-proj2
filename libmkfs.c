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
	root.next==false;
	inode root_node;
	inode free_data;
	free_data.ptrs[0]=1;	// Next open page....
	root_node.ptrs[0]=0;
	root_node.size=sizeof(dirent);
	memcpy(get_root_start(), (char*)&root, sizeof(dirent));
	memcpy(get_inode(0), (char*)&root_node, sizeof(inode));
	memcpy(get_inode(1), (char*)&free_data, sizeof(inode));
	//write("/", (char*)&d, sizeof(d), 0);
	//readdir("/");
	//mknod("/two.txt", 755);
	//readdir("/");
	pages_free();
}

