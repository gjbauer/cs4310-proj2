#include "inode.h"
#include "directory.h"
#include "hash.h"
#include "mfs.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int tree_lookup(const char* path, int i) {
	inode *root = get_inode(0);
	
	inode *ptr;
	
	dirent file;
	
	int level = count_l(path);
	
	printf("Level of file in directory structure: %d\n", level);
	
	memcpy((char*)&file, get_root_start(), sizeof(dirent));
	
	printf("Dirent name: %s\n", file.name);
	
	ptr = root;
	
	for (int i=0; i<level; i++) {
		memcpy((char*)&file, get_data(ptr->ptrs[i%2]), sizeof(dirent));
		printf("current search target: %s\n", split(path, i));
		printf("current file: %s\n", file.name);
		printf("inum: %d\n", ptr->inum);
		printf("i % 2: %d\n", i%2);
		if (!strcmp(file.name, split(path, i))) ptr = get_inode(file.inum);
		if ( (i%2) == 0 ) ptr = get_inode(ptr->iptr);
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

