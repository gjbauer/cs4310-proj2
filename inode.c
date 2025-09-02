#include "pages.h"
#include "bitmap.h"
#include "inode.h"
#include "hash.h"
#include "directory.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

//void print_inode(inode* node) {}

inode* get_inode(int inum) {
	void *ptr = get_inode_start();
	return (void*)((inode*)ptr + inum);
}

int
inode_find(const char *path) {
	void* ptr = (void*)get_inode_bitmap();
	for (int i=2; i<512; i++) {
		if (bitmap_get(ptr, i)==0) {
			return i;
		}
	}
	return alloc_inode(path);
}

int
alloc_inode(const char *path) {
	char *hpath;
	static char str[DIR_NAME];
	void* ibm = get_inode_bitmap();
	if (bitmap_get(ibm, hash(path))==1 || hash(path)==1 || hash(path)==0) {	// TODO: Fix this! We need the feature, but it is overflowing the stack...
		strncpy(str, path, DIR_NAME);
		str[strlen(path)-1] = hash(path);
		return alloc_inode(str);
	} else {
		bitmap_put(ibm, hash(path), 1);
		inode dd;
		memcpy((char*)&dd, (char*)get_inode(hash(path)), sizeof(inode));
		dd.ptrs[0]=alloc_page();
		dd.iptr=0;
		memcpy((char*)get_inode(hash(path)), (char*)&dd, sizeof(inode));
		return hash(path);
	}
}

int grow_inode(inode* node, int size)
/* This function was written by DeepSeek. Un-edited. */
{
    if (size <= node->size) {
        return 0; // Nothing to do
    }
    
    // Calculate how many pages we need now vs before
    int current_pages = bytes_to_pages(node->size);
    int needed_pages = bytes_to_pages(size);
    
    if (needed_pages <= current_pages) {
        node->size = size;
        return 0; // Already have enough pages
    }
    
    // Allocate new pages if needed
    for (int i = current_pages; i < needed_pages; i++) {
        int pnum = alloc_page();
        if (pnum < 0) {
            return -ENOSPC; // No free pages available
        }
        
        if (i < 2) {
            // Use direct pointers
            node->ptrs[i] = pnum;
        } else {
            // Use indirect pointer
            if (node->iptr == 0) {
                // Allocate indirect page if not already allocated
                node->iptr = alloc_page();
                if (node->iptr < 0) {
                    return -ENOSPC;
                }
                
                // Initialize indirect page to zeros
                int* indirect_page = pages_get_page(node->iptr);
                memset(indirect_page, 0, 4096);
            }
            
            // Get the indirect page and store the page number
            int* indirect_page = pages_get_page(node->iptr);
            int indirect_index = i - 2;
            
            if (indirect_index >= 1024) {
                free_page(pnum); // Free the page we just allocated
                return -EFBIG; // File too large (max 2 + 1024 pages)
            }
            
            indirect_page[indirect_index] = pnum;
        }
    }
    
    // Update the file size
    node->size = size;
    return 0;
}

int shrink_inode(inode* node, int size)
/* This function was written by DeepSeek. Un-edited. */
{
    if (size >= node->size) {
        return 0; // Nothing to do
    }
    
    // Calculate how many pages we need now vs before
    int current_pages = bytes_to_pages(node->size);
    int needed_pages = bytes_to_pages(size);
    
    if (needed_pages >= current_pages) {
        node->size = size;
        return 0; // Still need the same number of pages
    }
    
    // Free excess pages
    for (int i = current_pages - 1; i >= needed_pages; i--) {
        int pnum = -1;
        
        if (i < 2) {
            // Direct pointer
            pnum = node->ptrs[i];
            node->ptrs[i] = 0;
        } else {
            // Indirect pointer
            if (node->iptr == 0) {
                continue; // No indirect page allocated
            }
            
            int* indirect_page = pages_get_page(node->iptr);
            int indirect_index = i - 2;
            
            if (indirect_index >= 1024) {
                continue; // Invalid index
            }
            
            pnum = indirect_page[indirect_index];
            indirect_page[indirect_index] = 0;
        }
        
        if (pnum > 0) {
            free_page(pnum);
        }
    }
    
    // Free the indirect page if it's now empty
    if (node->iptr != 0 && needed_pages <= 2) {
        int* indirect_page = pages_get_page(node->iptr);
        int indirect_empty = 1;
        
        // Check if all entries in the indirect page are zero
        for (int i = 0; i < 1024; i++) {
            if (indirect_page[i] != 0) {
                indirect_empty = 0;
                break;
            }
        }
        
        if (indirect_empty) {
            free_page(node->iptr);
            node->iptr = 0;
        }
    }
    
    // Update the file size
    node->size = size;
    return 0;
}

int inode_get_pnum(inode* node, int fpn)
{
	return node->ptrs[fpn];
}
