#include <bsd/string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mkfs.h"
#include "mfs.h"

char *split(const char *path, int n) {
	int rv=0;
	char splt[DIR_NAME];
	if (n==0) {
		strcpy(splt, "/");
	} else {
		int c=0, i=0;
		for (; path[i] && c<n+1; i++) {
			splt[i]=path[i];
			if (path[i]=='/') c++;
		}
		if (splt[i-1]=='/') splt[i-1]='\0';
	}
	char *buf = (char*)calloc(DIR_NAME, sizeof(char));
	strncpy(buf, splt, DIR_NAME);
	return buf;
}

int
count_l(const char *path) {
	int c=0;
	for(int i=0; path[i]; i++) {
		if (path[i]=='/') c++;
	}
	return c;
}

int
find_parent(const char *path)
{
	char *ptr;
	int k = count_l(path);
	int n=0;
	for (int i=0; i<k; i++) {
		ptr = split(path, i);
		n = tree_lookup(ptr);
		if (n<0) return -ENOENT;
	}
	return n;
}

char *get_data(int offset)
{
	return ((char*)get_root_start()+offset);
}

/*int
readdir(const char *path)
{
	int rv = 0;
	char *ppath = split(path, count_l(path)-1);
	
	int l = tree_lookup(ppath);
	inode *n = get_inode(l);
	
	dirent file;
	
	while (true) {
		memcpy((char*)&file, get_data(n->ptrs[rv%2]), sizeof(dirent));
		if (file.name[0]!='/' || ( n->ptrs[rv%2] == 0 && rv > 0 ) ) {
			break;
		}
		rv++;
		printf("%s\n", file.name);
		if ( ( rv % 2 ) == 0 ) n = get_inode(n->iptr);
	}

	printf("readdir(%d)\n", rv);
	return rv;
}*/

int
mknod(const char *path, int mode)
{
	int rv = 0;
	char *ppath = split(path, count_l(path)-1);
	int l = inode_find(ppath);
	inode *dd = get_inode(l);
	
	int inum = alloc_inode(path);
	inode fn;
	memcpy((char*)&fn, (char*)get_inode(inum), sizeof(inode));
	fn.mode=mode;
	memcpy((char*)get_inode(inum), (char*)&fn, sizeof(inode));
	
	directory_put(dd, path, inum);

	printf("mknod(%s) -> %d\n", path, rv);
	return rv;
}

int
write(const char *path, const char *buf, size_t size, off_t offset)
{
	//printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	//return rv;
}

int
read(const char *path, char *buf, size_t size, off_t offset)
{
	int rv = 4096;
	printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}
