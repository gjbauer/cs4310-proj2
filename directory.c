#include "inode.h"
#include "directory.h"
#include <string.h>
#include <stdlib.h>

//void directory_init() {}
//int directory_lookup(inode* dd, const char* name) {}
int tree_lookup(const char* path) {
	size_t* count = (size_t*)get_root_start();
	dirent *ent = (dirent*)get_root_start()+1;
	for (int i=0; i<*count; i++) {
		if (!strcmp(ent->name, path)) return ent->inum;
		*ent++;
	}
	return -1;
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
