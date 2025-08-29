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
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]+5);	// Data pages start at 5
	
	for (int i=0;;i++) {
		if (!strncmp(file->name, name, DIR_NAME)) return file->inum;
		if (file->next==false) break;
		if ( (i%2) == 0 ) ptr = get_inode(ptr->iptr);
	}
}

int tree_lookup(const char* path) {
	// TODO: Rewrite this again more or less from scratch
	inode *root = get_inode(0);
	
	inode *ptr;
	
	ptr = root;
	
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]+5);	// Data pages start at 5
	
	int level = count_l(path);
	
	//printf("Level of file in directory structure: %d\n", level);
	
	
	for (int i=0; i<level; i++) {
		//printf("current search target: %s\n", split(path, i));
		printf("current file: %s\n", file->name);
		//printf("inum: %d\n", ptr->inum);
		//printf("i % 2: %d\n", i%2);
		if (!strcmp(file->name, split(path, i))) {
			
			ptr = get_inode(file->inum);
			
			break;
		}
		if (file->next==true) file++;
		//if ( (i%2) == 0 ) ptr = get_inode(ptr->iptr);
	}
	
	//printf("searching for main file...\n");
	
	/*for (int i=0; i<500; i++) {
		if (i % 2 == 0 && i > 0 ) {
			ptr = get_inode(ptr->iptr);
		}
		//printf("current file: %s\n", file.name);
		//printf("current search target: %s\n", path);
		file++;
		if (!strncmp(path, file->name, DIR_NAME)) {
			//printf("returning inum %d\n", file.inum);
			return file->inum;
		}
	}*/

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
	int inum = tree_lookup(path);
	inode *ptr = get_inode(inum);
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]+5);	// Data pages start at 5
	
	slist *dirlist;
	char *data = (char*)malloc(2 * (DIR_NAME+1) * sizeof(char));	// DIR_NAME+1 to include our delimiter ;)
	
	//printf("size of dirent : %d\n", sizeof(dirent));
	
	//printf("page size / size of dirent : %d\n", 4096/sizeof(dirent));
	
	// TODO : Keep track of how many files we have covered to know when to get the next page....
	
	for (int i=0, count=0 ;; i++, count++) {
		if (i % 2 == 0)
		{
			char *temp = (char*)malloc(i * (DIR_NAME+1) * sizeof(char));
			strncpy(temp, data, i * (DIR_NAME+1));
			data = (char*)realloc(data, i+2 * (DIR_NAME+1) * sizeof(char));
		}
		strncat(data, file->name, DIR_NAME);
		strncat(data, ";", 1);					// Choose a delimiter...I think a semicolon ( ; ) will work...
		if (file->next==false) break;
		else if ( count == 4096/sizeof(dirent) ) file = (dirent*)pages_get_page(ptr->ptrs[1]+5);	// Data pages start at 5
		else file++;
		if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			ptr = get_inode(ptr->iptr);
		}
	}
	
	dirlist = s_split(data, ';');
	
	return dirlist;
}

void print_directory(inode* dd)
{
	// TODO: Implement a function which prints the files in a directory to stdout
}


