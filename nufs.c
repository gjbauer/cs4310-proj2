// based on cs3650 starter code

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <bsd/string.h>
#include <assert.h>
#include <stdlib.h>

#define FUSE_USE_VERSION 26
#include <fuse.h>

#include "directory.h"
#include "inode.h"
#include "string.h"
#include "bitmap.h"

// implementation for: man 2 access
// Checks if a file exists.
int
nufs_access(const char *path, int mask)
{
	int rv = 0;
	int l = tree_lookup(path);
	if ( l == -ENOENT ) rv = -1;
	printf("access(%s, %04o) -> %d\n", path, mask, rv);
	return rv;
}

// implementation for: man 2 stat
// gets an object's attributes (type, permissions, size, etc)
int
nufs_getattr(const char *path, struct stat *st)
/* This function was written by DeepSeek. */
{
	int rv = 0;
	memset(st, 0, sizeof(struct stat));
	
	// Handle root directory
	if (strcmp(path, "/") == 0) {
		st->st_mode = S_IFDIR | 0755;
		st->st_nlink = 2;
		st->st_size = 0;
		st->st_uid = getuid();
		st->st_gid = getgid();
		st->st_atime = st->st_mtime = st->st_ctime = time(NULL);
		printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode, st->st_size);
		return 0;
	}
	
	// Look up the inode for this path
	int inum = tree_lookup(path);
	if (inum < 0) {
		rv = -ENOENT;
	}
	
	// Get the inode
	inode* node = get_inode(inum);
	if (node == NULL) {
		rv = -ENOENT;
	}
	
	// Fill in the stat structure
	st->st_mode = node->mode;
	st->st_nlink = 1;
	st->st_size = node->size;
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = st->st_mtime = st->st_ctime = time(NULL);
	st->st_ino = inum;
	
	printf("getattr(%s) -> (%d) {mode: %04o, size: %ld}\n", path, rv, st->st_mode, st->st_size);
	return rv;
}

char *partial_path(char *path)
{
	char *partial = (char*)malloc(DIR_NAME * sizeof(char));
	
	int i,j;
	for (i=0, j=0; i < DIR_NAME && j<count_l(path); i++)
	{
		if (path[i] == '/') j++;
	}
	for (int k=0; k<DIR_NAME && path[i]!='\0'; k++, i++) partial[k] = path[i];
	
	return partial;
}

// implementation for: man 2 readdir
// lists the contents of a directory
int
nufs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
/* This function was originally written by DeepSeek.
 * Finished by myself. */
{
	int rv = 0;
	
	// Get the directory listing
	slist* entries = directory_list(path);
	slist* current = entries;
	
	// Add "." and ".." entries first
	struct stat st;
	
	// Add all directory entries
	while (current != NULL) {
		char *relative = partial_path(current->data);
		
		// Get file attributes
		rv = nufs_getattr(current->data, &st);
		if (rv == 0) {
			if (!strcmp(current->data, "/")) filler(buf, ".", &st, 0);
			else filler(buf, relative, &st, 0);
		}
		
		free(relative);
		
		current = current->next;
	}
	
	// Free the list
	s_free(entries);
	
	printf("readdir(%s) -> %d\n", path, rv);
	return rv;
}

// mknod makes a filesystem object like a file or directory
// called for: man 2 open, man 2 link
int
nufs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int rv = 0;
	char *ppath = split(path, count_l(path)-1);
	int l = tree_lookup(ppath);
	inode *dd = get_inode(l);
	
	int inum = alloc_inode(path);
	inode fn;
	memcpy((char*)&fn, (char*)get_inode(inum), sizeof(inode));
	if (mode < 10000) mode = mode | 0100000;		// Regular file
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
	fn.refs=0;
	fn.ptrs[0]=alloc_page()-5;
	fn.ptrs[1]=alloc_page()-5;
	memcpy((char*)get_inode(inum), (char*)&fn, sizeof(inode));
	
	directory_put(dd, path, inum);

	printf("mknod(%s, %04o) -> %d\n", path, mode, rv);
	return rv;
}

// most of the following callbacks implement
// another system call; see section 2 of the manual
int
nufs_mkdir(const char *path, mode_t mode)
{
	int rv = nufs_mknod(path, mode | 040000, 0);
	printf("mkdir(%s) -> %d\n", path, rv);
	return rv;
}

int
nufs_unlink(const char *path)
/* This function was originally written by DeepSeek.
 * Finished by myself. */
{
	
	// Look up parent directory
	int dir_inum = tree_lookup(split(path, count_l(path)-1));
	if (dir_inum < 0) {
		return -ENOENT;
	}
	
	inode* dir_node = get_inode(dir_inum);
	
	// Look up the file
	int file_inum = directory_lookup(dir_node, path);
	if (file_inum < 0) {
		return -ENOENT;
	}
	
	inode* file_node = get_inode(file_inum);
	if (file_node == NULL) {
		return -ENOENT;
	}
	
	// Remove directory entry
	int rv = directory_delete(dir_node, path);
	if (rv < 0) {
		return rv;
	}
	
	// Decrement reference count and free if needed
	file_node->refs--;
	if (file_node->refs <= 0) {
		// Free all data pages
		for (int i = 0; i < 2; i++) {
			if (file_node->ptrs[i] != 0) {
				free_page(file_node->ptrs[i]);
			}
		}
		
		/*// Free indirect page	<- That's wrong, DeepSeek. This should be a loop which frees all iptr inodes...can implement later...
		if (file_node->iptr != 0) {
			free_page(file_node->iptr);
		}*/
		
		// Free inode
		void* ibm = get_inode_bitmap();
		bitmap_put(ibm, file_inum, 0);
	}
	
	printf("unlink(%s) -> %d\n", path, rv);
	return rv;
}

int
nufs_link(const char *from, const char *to)
{
	int rv = 0;
	int l = tree_lookup(from);
	if ( l == -ENOENT ) rv = -ENOENT;
	inode *parent = get_inode(tree_lookup(split(to, count_l(to)-1)));
	
	directory_put(parent, to, l);
	printf("link(%s => %s) -> %d\n", from, to, rv);
	return rv;
}

int
nufs_rmdir(const char *path)
{
	int rv = -1;
	printf("rmdir(%s) -> %d\n", path, rv);
	return rv;
}

// implements: man 2 rename
// called to move a file within the same filesystem
int
nufs_rename(const char *from, const char *to)
{
	int rv = -1;
	int l = tree_lookup(split(from, count_l(from)-1));
	inode *dd = get_inode(l);
	
	directory_delete(dd, from);
	l = tree_lookup(from);
	directory_put(dd, to, l);
	
	printf("rename(%s => %s) -> %d\n", from, to, rv);
	return rv;
}

int
nufs_chmod(const char *path, mode_t mode)
{
	int rv = -1;
	int l = tree_lookup(path);
	inode dd;
	memcpy((char*)&dd, (char*)get_inode(l), sizeof(inode));
	dd.mode = mode;
	memcpy((char*)get_inode(l), (char*)&dd, sizeof(inode));
	printf("chmod(%s, %04o) -> %d\n", path, mode, rv);
	return rv;
}

int
nufs_truncate(const char *path, off_t size)
{
	int rv = -1;
	printf("truncate(%s, %ld bytes) -> %d\n", path, size, rv);
	return rv;
}

// this is called on open, but doesn't need to do much
// since FUSE doesn't assume you maintain state for
// open files.
int
nufs_open(const char *path, struct fuse_file_info *fi)
{
	int rv = 0;
	printf("open(%s) -> %d\n", path, rv);
	return rv;
}

// Actually read data
int
nufs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
/* This function was written by DeepSeek. */
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
	
	printf("read(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, bytes_read);
	return bytes_read;
}

// Actually write data
int
nufs_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
/* This function was written by DeepSeek. */
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
	
	printf("write(%s, %ld bytes, @+%ld) -> %d\n", path, size, offset, bytes_written);
	return bytes_written;
}

// Update the timestamps on a file or directory.
int
nufs_utimens(const char* path, const struct timespec ts[2])
{
	int rv = -1;
	printf("utimens(%s, [%ld, %ld; %ld %ld]) -> %d\n",
		   path, ts[0].tv_sec, ts[0].tv_nsec, ts[1].tv_sec, ts[1].tv_nsec, rv);
	return rv;
}

// Extended operations
int
nufs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi,
		   unsigned int flags, void* data)
{
	int rv = -1;
	printf("ioctl(%s, %d, ...) -> %d\n", path, cmd, rv);
	return rv;
}

void
nufs_init_ops(struct fuse_operations* ops)
{
	memset(ops, 0, sizeof(struct fuse_operations));
	ops->access   = nufs_access;
	ops->getattr  = nufs_getattr;
	ops->readdir  = nufs_readdir;
	ops->mknod	= nufs_mknod;
	ops->mkdir	= nufs_mkdir;
	ops->link	 = nufs_link;
	ops->unlink   = nufs_unlink;
	ops->rmdir	= nufs_rmdir;
	ops->rename   = nufs_rename;
	ops->chmod	= nufs_chmod;
	ops->truncate = nufs_truncate;
	ops->open	  = nufs_open;
	ops->read	 = nufs_read;
	ops->write	= nufs_write;
	ops->utimens  = nufs_utimens;
	ops->ioctl	= nufs_ioctl;
};

struct fuse_operations nufs_ops;

int
main(int argc, char *argv[])
{
	assert(argc > 2 && argc < 7);
	storage_init(argv[--argc]);
	nufs_init_ops(&nufs_ops);
	return fuse_main(argc, argv, &nufs_ops, NULL);
}

