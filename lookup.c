#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "inode.h"
#include "directory.h"
#include "pages.h"

int _tree_lookup(const char* path) {
	if (!strcmp(path, "/")) return 0;
	inode *n = get_inode(0);
	dirent *p0, *p1;
lookup_loop:
	p0 = (dirent*)((char*)get_root_start()+n->ptrs[0]);
	printf("%s\n", p0->name);
	p1 = (dirent*)((char*)get_root_start()+n->ptrs[1]);
	printf("%s\n", p1->name);
	if (!strcmp(p0->name, path)) {
		return p0->inum;
	} else if (!strcmp(p0->name, "*")) {
		return -ENOENT;
	} else if (!strcmp(p1->name, path)) {
		return p1->inum;
	} else if (!strcmp(p1->name, "*")) {
		return -ENOENT;
	} else {
		n = get_inode(n->iptr);
		goto lookup_loop;
	}
	return -ENOENT;
}

int main () {
	storage_init("data.nufs");
	printf("%d\n", _tree_lookup("/hello.txt"));
	pages_free();
}
