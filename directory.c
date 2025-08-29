#include "inode.h"
#include "directory.h"
#include "hash.h"
#include "mfs.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int directory_lookup(inode* dd, const char* name)
{
	inode *ptr = dd;
	dirent file;
	
	for (int i=0;;i++) {
		memcpy((char*)&file, get_data(ptr->ptrs[i%2]), sizeof(dirent));
		if (!strncmp(file.name, name, DIR_NAME)) return file.inum;
		if (file.next==NULL) break;
		if ( (i%2) == 0 ) ptr = get_inode(ptr->iptr);
	}
}

int tree_lookup(const char* path) {
	inode *root = get_inode(0);
	
	inode *ptr;
	
	dirent file;
	
	int level = count_l(path);
	
	//printf("Level of file in directory structure: %d\n", level);
	
	ptr = root;
	
	for (int i=0; i<level; i++) {
		memcpy((char*)&file, get_data(ptr->ptrs[i%2]), sizeof(dirent));
		//printf("current search target: %s\n", split(path, i));
		//printf("current file: %s\n", file.name);
		//printf("inum: %d\n", ptr->inum);
		//printf("i % 2: %d\n", i%2);
		if (!strcmp(file.name, split(path, i))) {
			dirent *temp = (dirent*)get_data(ptr->ptrs[i%2]);
			
			ptr = get_inode(temp->inum);
			
			break;
		}
		if ( (i%2) == 0 ) ptr = get_inode(ptr->iptr);
	}
	
	//printf("searching for main file...\n");
	
	for (int i=0; i<500; i++) {
		if (i % 2 == 0 && i > 0 ) {
			ptr = get_inode(ptr->iptr);
		}
		//printf("current file: %s\n", file.name);
		//printf("current search target: %s\n", path);
		memcpy((char*)&file, get_data(ptr->ptrs[i%2]), sizeof(dirent));
		if (!strncmp(path, file.name, DIR_NAME)) {
			//printf("returning inum %d\n", file.inum);
			return file.inum;
		}
	}

	//printf("returning -ENOENT\n");
	return -ENOENT;
}
int directory_put(inode* dd, const char* name, int inum)
{
	// Setup file
	dirent file;
	strncpy(file.name, name, DIR_NAME);
	file.inum = inum;
	
	return 0;
}

int directory_delete(inode* dd, const char* name)
{
	// TODO: Delete function
}

slist* directory_list(const char* path)
{
	// TODO: Implement a function which lists all files in a directory in an slist
	int inum = tree_lookup(path);
	inode *ptr = get_inode(inum);;
	dirent file;
	
	slist *dirlist = (slist*)malloc(sizeof(slist));
	dirlist->data = (char*)malloc(2 * (DIR_NAME+1) * sizeof(char));	// DIR_NAME+1 to include our delimiter ;)
	
	for (int i=0;; i++) {
		memcpy((char*)&file, get_data(ptr->ptrs[i%2]), sizeof(dirent));
		if (i % 2 == 0)
		{
			// TODO: Grow our dynamic array
			char *data = (char*)malloc(i * (DIR_NAME+1) * sizeof(char));
			strncpy(data, dirlist->data, i * (DIR_NAME+1));
			free(dirlist->data);
			dirlist->data = (char*)malloc(i+2 * (DIR_NAME+1) * sizeof(char));
		}
		strncat(dirlist->data, file.name, DIR_NAME);
		strncat(dirlist->data, ";", 1); // TODO: Choose a delimiter...I think a semicolon ( ; ) will work...
		if (file.next==NULL) break;
		if ( (i%2) == 0 ) ptr = get_inode(ptr->iptr);
	}
	return dirlist;
}

void print_directory(inode* dd)
{
	// TODO: Implement a function which prints the files in a directory to stdout
}


