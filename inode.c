#include "pages.h"
#include "bitmap.h"
#include "inode.h"
#include "hash.h"
#include "directory.h"
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

void print_inode(inode* node)
{
	printf("Printing inode information...\n");
	printf("refs = %d\n", node->refs);
	printf("mode = %d\n", node->mode);
	printf("size = %d\n", node->size);
	for (int i=0; i<2; i++) printf("ptrs[%d] = %d\n", i, node->ptrs[i]);
	printf("iptr = %d\n", node->iptr);
	printf("inum = %d\n", node->inum);
	printf("Inode printing complete!\n");
}

inode* get_inode(int inum) {
	void *ptr = get_inode_start();
	return (void*)((inode*)ptr + inum);
}

int
alloc_inode() {
	void* ibm = get_inode_bitmap();
	inode *node;
	for (int ii = 0; ii < 4096*64; ++ii) {
		if (!bitmap_get(ibm, ii)) {
			bitmap_put(ibm, ii, 1);
			node = get_inode(ii);
			node->ptrs[0]=0;
			node->ptrs[1]=1;
			node->size=0;
			node->iptr=0;
			printf("+ alloc_inode() -> %d\n", ii);
			return ii;
		}
	}

	return -1;
}

void
free_inode(int inum) {
	printf("+ free_inode(%d)\n", inum);
	void* ibm = get_inode_bitmap();
	bitmap_put(ibm, inum, 0);
}

int grow_inode(inode* node, int size)
{
	int current_pages = bytes_to_pages(node->size);
	int needed_pages = bytes_to_pages(size);
	inode *temp = node;
	
	if (needed_pages <= current_pages) {
		node->size = size;
		return 0;
	}
	
	for (int i=0; i<current_pages; i++)
	{
		if (i%2==0&&i>0) temp = get_inode(temp->iptr);
	}
	
	for (int i=current_pages ; i < needed_pages; i++)
	{
		if (i%2==0&&i>0)
		{
			temp->iptr = alloc_inode();
			temp = get_inode(temp->iptr);
		}
		temp->ptrs[i%2] = alloc_page();
	}
	
	node->size = size;
	return 0;
}

int shrink_inode(inode* node, int size)
{
	int current_pages = bytes_to_pages(node->size);
	int needed_pages = bytes_to_pages(size);
	inode *temp = node;
	
	if (needed_pages == current_pages) {
		node->size = size;
		return 0;
	}
	
	for (int i=0; i<needed_pages; i+=2)
	{
		if (i>0) temp = get_inode(temp->iptr);
	}
	
	inode *last = node;
	
	for (int j=0, i=needed_pages ; i < current_pages; i++, j++)
	{
		if (j%2==0&&j>0)
		{
			free_inode(temp->iptr);
			temp = get_inode(temp->iptr);
			j=0;
		}
		free_page(temp->ptrs[j]);
	}
	
	node->size = size;
	return 0;
}

int inode_get_pnum(inode* node, int fpn)
{
	return node->ptrs[fpn];
}
