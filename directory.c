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
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]+5);	// Data pages start at 5
	
	for (int count=0;;count++) {
		if (!strncmp(file->name, name, DIR_NAME)) return file->inum;
		if (file->next==false) break;
		else if ( count == 4096/sizeof(dirent) ) file = (dirent*)pages_get_page(ptr->ptrs[1]+5);	// Data pages start at 5
		else if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			ptr = get_inode(ptr->iptr);
		}
		else file++;
	}
	
	return -ENOENT;
}

int tree_lookup(const char* path) {
	inode *root = get_inode(0);
	inode *ptr = root;
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]+5);	// Data pages start at 5
	int level = count_l(path);
	
	for (int i=0; i<level; i++) {
		int inum = directory_lookup(ptr, split(path, i));
		if (inum == -ENOENT) return -ENOENT;
		ptr = get_inode(inum);
	}
	
	return directory_lookup(ptr, path);
}

int directory_put(inode* dd, const char* name, int inum)
{
	// Setup file
	dirent file;
	strncpy(file.name, name, DIR_NAME);
	file.inum = inum;
	file.active = true;
	
	dirent *ptr = (dirent*)pages_get_page(dd->ptrs[0]+5);
	
	for (int count=0 ;; count++)
	{
		if ( count == 4096/sizeof(dirent) ) ptr = (dirent*)pages_get_page(dd->ptrs[1]+5);	// Data pages start at 5
		else if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			if (dd->iptr=0) dd->iptr=alloc_inode(".");
			dd = get_inode(dd->iptr);
			ptr = (dirent*)pages_get_page(dd->ptrs[0]+5);
		}
		else if (ptr->next == false ) {
			ptr->next=true;
			ptr++;
			break;
		}
		else ptr++;
	}
	
	memcpy((char*)ptr, (char*)&file, sizeof(dirent));
	
	return 0;
}

int directory_delete(inode* dd, const char* name)
{	
	dirent *ptr = (dirent*)pages_get_page(dd->ptrs[0]+5);
	
	for (int count=0 ;; count++)
	{
		if ( count == 4096/sizeof(dirent) ) ptr = (dirent*)pages_get_page(dd->ptrs[1]+5);	// Data pages start at 5
		else if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			dd = get_inode(dd->iptr);
			ptr = (dirent*)pages_get_page(dd->ptrs[0]+5);
		}
		else if ( !strcmp((ptr+1)->name, name) ) {
			(ptr+1)->active=false;
			ptr->next = (ptr+1)->next;
			inode dd;
			memcpy((char*)&dd, get_inode((ptr+1)->inum), sizeof(inode));
			dd.refs -= 1;
			memcpy(get_inode((ptr+1)->inum), (char*)&dd, sizeof(inode));
			if ( dd.refs == 0 ) bitmap_put(get_inode_bitmap(), dd.inum, 0);
			break;
		}
		else ptr++;
	}

	return 0;
}

slist* directory_list(const char* path)
{
	int inum = tree_lookup(path);
	inode *ptr = get_inode(inum);
	dirent *file = (dirent*)pages_get_page(ptr->ptrs[0]+5);	// Data pages start at 5
	slist *dirlist;
	char *data = (char*)malloc(2048 * (DIR_NAME+1) * sizeof(char));	// DIR_NAME+1 to include our delimiter ;)
	
	for (int i=0, count=0 ;; i++, count++) {
		if (file->active) strncat(data, file->name, DIR_NAME);
		strncat(data, ";", 1);					// Choose a delimiter...I think a semicolon ( ; ) will work...
		if (file->next==false) break;
		else if ( count == 4096/sizeof(dirent) ) file = (dirent*)pages_get_page(ptr->ptrs[1]+5);	// Data pages start at 5
		else file++;
		if ( count == 8192/sizeof(dirent) ) {
			count = 0;
			ptr = get_inode(ptr->iptr);
			file = (dirent*)pages_get_page(ptr->ptrs[0]+5);
		}
	}
	
	dirlist = s_split(data, ';');
	
	return dirlist;
}

void print_directory(inode* dd)
{
	dirent *ptr = (dirent*)pages_get_page(dd->ptrs[0]+5);
	
	for (int count=0 ;; count++)
	{
		if ( ptr->next == false ) {
			printf("%s\n", ptr->name);
			break;
		}
		else if ( count == 4096/sizeof(dirent) ) {
			printf("%s\n", ptr->name);
			ptr = (dirent*)pages_get_page(dd->ptrs[1]+5);	// Data pages start at 5
		}
		else if ( count == 8192/sizeof(dirent) ) {
			printf("%s\n", ptr->name);
			count = 0;
			dd = get_inode(dd->iptr);
		}
		else {
			printf("%s\n", ptr->name);
			ptr++;
		}
	}
}


