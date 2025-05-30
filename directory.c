#include "inode.h"
#include "directory.h"
#include "hash.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int tree_lookup(const char* path, int i) {
	int k = count_l(path);
	
	for (int i=0; i<k; i++);
	

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

