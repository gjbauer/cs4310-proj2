#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mfs.h"
#include "mkfs.h"
#include <stdlib.h>


int mkstop() {
    int rv = 0;
    dirent data;
    data.inum=0;
    strcpy(data.name, "*");
    data.active=true;
    write("/", (char*)&data, sizeof(data), 0);
    printf("mkstop(%s) -> %d\n", "*", rv);
    return rv;
}

void
mkfs() {
	pages_init("data.nufs");
	mkdir("/", 755);
	pages_free();
}

