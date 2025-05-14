// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME 48

#define FILE 'F'
#define DIRECTORY 'D'

//#include "slist.h"
#include "pages.h"
#include "inode.h"
#include <stdbool.h>

typedef struct dirent {
    char name[DIR_NAME];
    int  inum;
    int count;
    char type;
    bool active;
    struct dirent *parent;
    struct dirent *next;
    //char _reserved[11];
} dirent;

//void directory_init();
//int directory_lookup(inode* dd, const char* name);
int tree_lookup(const char* path, int i);
//int directory_put(inode* dd, const char* name, int inum);
//int directory_delete(inode* dd, const char* name);
//slist* directory_list(const char* path);
//void print_directory(inode* dd);

#endif

