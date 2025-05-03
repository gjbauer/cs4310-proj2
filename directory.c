#include "inode.h"
#include "directory.h"
#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int tree_lookup(const char* subpath, int i) {
	if (!strcmp(subpath, "/")) return 0;
	inode *n = get_inode(i);
	dirent *p0, *p1;
lookup_loop:
	p0 = (dirent*)((char*)get_root_start()+n->ptrs[0]);
	p1 = (dirent*)((char*)get_root_start()+n->ptrs[1]);
	if (!strcmp(p0->name, subpath)) {
		return p0->inum;
	} else if (!strcmp(p0->name, "*")) {
		return -ENOENT;
	} else if (!strcmp(p1->name, subpath)) {
		return p1->inum;
	} else if (!strcmp(p1->name, "*")) {
		return -ENOENT;
	} else {
		n = get_inode(n->iptr);
		goto lookup_loop;
	}
	return -ENOENT;
}
int directory_put(inode* dd, const char* name, int inum) {
	/*dirent* d = malloc(sizeof(dirent*));
	strcpy(d->name, name);
	d->inum = inum;
	dirent *ent = (dirent*)get_root_start();
	while (ent) {
		if (ent->active==false) break;
		else *ent++;
	}
	if (!ent) return -1;
	memcpy(ent, &d, sizeof(d));*/
	return 0;
}

