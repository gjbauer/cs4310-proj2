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

#define SECTOR_SIZE 4096
#define INODE_SIZE 24 //128	<= DeepSeeks assumed size
#define DIRECT_BLOCKS 12

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
mkdir(const char *path, mode_t mode)
{
    	int rv = mknod(path, mode | 040000);
	printf("mkdir(%s) -> %d\n", path, rv);
	return rv;
}

int
mknod(const char *path, int mode)
{
	int rv = 0;
	char *ppath = split(path, count_l(path)-1);
	int l = tree_lookup(ppath);
	inode *dd = get_inode(l);
	
	int inum = alloc_inode(path);
	inode fn;
	memcpy((char*)&fn, (char*)get_inode(inum), sizeof(inode));
	if (mode < 10000) mode = mode | 070000;		// Regular file
	else {			// Directory
		dirent *ptr = (dirent*)pages_get_page(get_inode(inum)->ptrs[0]+5);
		strcpy(ptr->name, ".");
		ptr->inum=inum;
		ptr->next=true;
		ptr++;
		strcpy(ptr->name, "..");
		ptr->inum=dd->inum;
	}
	fn.mode=mode;
	fn.refs=1;
	memcpy((char*)get_inode(inum), (char*)&fn, sizeof(inode));
	
	directory_put(dd, path, inum);

	printf("mknod(%s) -> %d\n", path, rv);
	return rv;
}

int storage_write(const char* path, const char* buf, size_t size, off_t offset)
/* This function was written by DeepSeek. Un-edited. */
{
    // Find the inode for the given path
    int inum = tree_lookup(path);
    if (inum < 0) {
        return -ENOENT; // File not found
    }
    
    inode* node = get_inode(inum);
    if (node == NULL) {
        return -ENOENT;
    }
    
    // If writing beyond current size, we need to grow the file
    if (offset + size > node->size) {
        int new_size = offset + size;
        int rv = grow_inode(node, new_size);
        if (rv < 0) {
            return -ENOSPC; // No space left
        }
    }
    
    int bytes_written = 0;
    int remaining = size;
    off_t current_offset = offset;
    
    while (remaining > 0) {
        // Calculate which page we're writing to
        int page_index = current_offset / 4096;
        int page_offset = current_offset % 4096;
        
        // Get the physical page number for this logical page
        int pnum = -1;
        
        if (page_index < 2) {
            // Direct pointer
            pnum = node->ptrs[page_index];
        } else {
            // Indirect pointer - need to read from the indirect page
            if (node->iptr == 0) {
                return -ENOSPC; // No indirect page allocated (should have been allocated by grow_inode)
            }
            
            // Read the page number from the indirect page
            int* indirect_page = pages_get_page(node->iptr);
            int indirect_index = page_index - 2;
            
            if (indirect_index >= 1024) {
                return -EFBIG; // File too large
            }
            
            pnum = indirect_page[indirect_index];
        }
        
        if (pnum <= 0) {
            return -ENOSPC; // Page not allocated (should have been allocated by grow_inode)
        }
        
        // Get pointer to the page data
        char* page_data = pages_get_page(pnum);
        
        // Calculate how much to write to this page
        int bytes_to_write = 4096 - page_offset;
        if (bytes_to_write > remaining) {
            bytes_to_write = remaining;
        }
        
        // Copy data from buffer to page
        memcpy(page_data + page_offset, buf + bytes_written, bytes_to_write);
        
        // Update counters
        bytes_written += bytes_to_write;
        remaining -= bytes_to_write;
        current_offset += bytes_to_write;
    }
    
    // Update file size if we wrote beyond the previous end
    if (offset + bytes_written > node->size) {
        node->size = offset + bytes_written;
    }
    
    return bytes_written;
}

int
write(const char *path, const char *buf, size_t size, off_t offset)
{
	//printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	//return rv;
}

int storage_read(const char* path, char* buf, size_t size, off_t offset)
/* This function was written by DeepSeek. Un-edited. */
{
    // Find the inode for the given path
    int inum = tree_lookup(path);
    if (inum < 0) {
        return -ENOENT; // File not found
    }
    
    inode* node = get_inode(inum);
    if (node == NULL) {
        return -ENOENT;
    }
    
    // Check if offset is beyond file size
    if (offset >= node->size) {
        return 0; // Nothing to read
    }
    
    // Adjust size if it would read beyond the end of the file
    if (offset + size > node->size) {
        size = node->size - offset;
    }
    
    if (size == 0) {
        return 0;
    }
    
    int bytes_read = 0;
    int remaining = size;
    off_t current_offset = offset;
    
    while (remaining > 0) {
        // Calculate which page we're reading from
        int page_index = current_offset / 4096;
        int page_offset = current_offset % 4096;
        
        // Get the physical page number for this logical page
        int pnum = -1;
        
        if (page_index < 2) {
            // Direct pointer
            pnum = node->ptrs[page_index];
        } else {
            // Indirect pointer - need to read from the indirect page
            if (node->iptr == 0) {
                break; // No indirect page allocated
            }
            
            // Read the page number from the indirect page
            int* indirect_page = pages_get_page(node->iptr);
            int indirect_index = page_index - 2;
            
            if (indirect_index >= 1024) {
                break; // Beyond maximum supported pages
            }
            
            pnum = indirect_page[indirect_index];
        }
        
        if (pnum <= 0) {
            break; // No page allocated here
        }
        
        // Get pointer to the page data
        char* page_data = pages_get_page(pnum);
        
        // Calculate how much to read from this page
        int bytes_to_read = 4096 - page_offset;
        if (bytes_to_read > remaining) {
            bytes_to_read = remaining;
        }
        
        // Copy data from page to buffer
        memcpy(buf + bytes_read, page_data + page_offset, bytes_to_read);
        
        // Update counters
        bytes_read += bytes_to_read;
        remaining -= bytes_to_read;
        current_offset += bytes_to_read;
    }
    
    return bytes_read;
}

int
read(const char *path, char *buf, size_t size, off_t offset)
{
	int rv = 4096;
	printf("INODE_SIZE : %d\n", sizeof(inode));
	printf("offset / 4096 : %d\n", offset/4096);
	printf("offset / 4096 % 2 : %d\n", (offset/4096) % 2);
	printf("size + offset / 4096 : %d\n", (size+offset)/4096);
	printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, rv);
	return rv;
}
