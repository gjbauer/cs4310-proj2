#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mfs.h"
#include "mkfs.h"
#include <stdlib.h>

void
mkfs() {
	pages_init("data.nufs");
	dirent root;
	strcpy(root.name, "/");
	root.inum = 0;
	root.next==false;
	root.parent==NULL;
	inode root_node;
	root_node.ptrs[0]=0;
	root_node.size[0]=sizeof(dirent);
	root_node.size[1]=0;
	memcpy(get_root_start(), (char*)&root, sizeof(dirent));
	memcpy(get_inode(0), (char*)&root_node, sizeof(inode));
	//write("/", (char*)&d, sizeof(d), 0);
	//readdir("/");
	//mknod("/two.txt", 755);
	//readdir("/");
	pages_free();
}

