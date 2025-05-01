#include "inode.h"
#include "directory.h"
#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

//void directory_init() {}
//int directory_lookup(inode* dd, const char* name) {}
int tree_lookup(const char* path) {
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
int directory_put(inode* dd, const char* name, int inum) {
	dirent* d = malloc(sizeof(dirent*));
	strcpy(d->name, name);
	d->inum = inum;
	dirent *ent = (dirent*)get_root_start();
	while (ent) {
		if (ent->active==false) break;
		else *ent++;
	}
	if (!ent) return -1;
	memcpy(ent, &d, sizeof(d));
	return 0;
}
//int directory_delete(inode* dd, const char* name) {}
//slist* directory_list(const char* path) {}
//void print_directory(inode* dd) {}
