#include "inode.h"
#include "directory.h"
#include "hash.h"
#include "bitmap.h"
#include "string.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int directory_lookup(inode* dd, const char* name)
{
	inode *ptr = dd;
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]);	
	
	for (int count=0;;count++) {
		if (!strncmp(file->name, name, DIR_NAME)) return file->inum;
		if (file->next==false) break;
		else if ( count == 4096/sizeof(dirent) ) file = (dirent*)pages_get_page(ptr->ptrs[1]);	
		else if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			ptr = get_inode(ptr->iptr);
			file = (dirent*)pages_get_page(ptr->ptrs[0]);
		}
		else file++;
	}
	
	return -ENOENT;
}

int tree_lookup(const char* path) {
	inode *root = get_inode(0);
	inode *ptr = root;
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]);	
	int level = count_l(path);
	
	for (int i=0; i<level; i++) {
		char *ppath = split(path, i);
		int inum = directory_lookup(ptr, ppath);
		free(ppath);
		if (inum == -ENOENT) return -ENOENT;
		ptr = get_inode(inum);
	}
	
	return directory_lookup(ptr, path);
}

int directory_put(inode* dd, const char* name, int inum)
{
	// Setup file
	dirent file;
	//strcpy(file.name, name);
	int i;
	for (i=0; name[i]!=0 && i<DIR_NAME; i++) file.name[i] = name[i];
	file.name[i] = 0;
	printf("name size = %d\n", i);
	printf("file.name = %s\n", file.name);
	file.inum = inum;
	file.active = true;
	
	dirent *ptr = (dirent*)pages_get_page(dd->ptrs[0]);
	
	for (int count=0 ;; count++)
	{
		if ( count == 4096/sizeof(dirent) )
		{
			if (dd->ptrs[1]==0) dd->ptrs[1] = alloc_page();
			ptr = (dirent*)pages_get_page(dd->ptrs[1]);
		}
		else if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			if (dd->iptr==0) dd->iptr=alloc_inode();
			dd = get_inode(dd->iptr);
			dd->ptrs[1] = alloc_page();
			ptr = (dirent*)pages_get_page(dd->ptrs[0]);
		}
		else if (ptr->next == false ) {
			ptr->next=true;
			ptr++;
			break;
		}
		else ptr++;
	}
	
	memcpy(ptr, &file, sizeof(dirent));
	
	return 0;
}

int directory_delete(inode* dd, const char* name)
{	
	dirent *ptr = (dirent*)pages_get_page(dd->ptrs[0]);
	dirent *prev = NULL;
	
	for (int count=0 ;; count++)
	{
		if ( count == 4096/sizeof(dirent) ) ptr = (dirent*)pages_get_page(dd->ptrs[1]);	
		else if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			dd = get_inode(dd->iptr);
			ptr = (dirent*)pages_get_page(dd->ptrs[0]);
		}
		else if ( !strcmp(ptr->name, name) ) {
			ptr->active=false;
			if (prev) prev->next = ptr->next;
			break;
		}
		else {
			prev = ptr;
			ptr++;
		}
	}

	return 0;
}

slist* directory_list(const char* path)
{
	int inum = tree_lookup(path);
	inode *ptr = get_inode(inum);
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]);	
	slist *dirlist;
	char *data = (char*)malloc(2048 * (DIR_NAME+1) * sizeof(char));	// DIR_NAME+1 to include our delimiter ;)
	
	for (int i=0, count=0 ;; i++, count++) {
		if (file->active==true) strncat(data, file->name, DIR_NAME);
		strncat(data, ";", 1);					// Choose a delimiter...I think a semicolon ( ; ) will work...
		if (file->next==false) break;
		else if ( count == 4096/sizeof(dirent) ) file = (dirent*)pages_get_page(ptr->ptrs[1]);	
		else file++;
		if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			ptr = get_inode(ptr->iptr);
			file = (dirent*)pages_get_page(ptr->ptrs[0]);
		}
	}
	
	dirlist = s_split(data, ';');
	
	return dirlist;
}

void print_directory(inode* dd)
{
	dirent *ptr = (dirent*)pages_get_page(dd->ptrs[0]);
	
	for (int count=0 ;; count++)
	{
		if ( ptr->next == false ) {
			printf("%s\n", ptr->name);
			break;
		}
		else if ( count == 4096/sizeof(dirent) ) {
			printf("%s\n", ptr->name);
			ptr = (dirent*)pages_get_page(dd->ptrs[1]);	
		}
		else if ( count == 8192/sizeof(dirent) ) {
			printf("%s\n", ptr->name);
			count = 0;
			dd = get_inode(dd->iptr);
			ptr = (dirent*)pages_get_page(dd->ptrs[0]);
		}
		else {
			printf("%s\n", ptr->name);
			ptr++;
		}
	}
}


