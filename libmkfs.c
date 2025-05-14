#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mfs.h"
#include "mkfs.h"
#include <stdlib.h>

void
mkfs() {
	pages_init("data.nufs");
	size_t* count = (size_t*)get_root_start();
	*count = 0;
	mknod("/", 755);
	readdir("/");
	mknod("/two.txt", 755);
	readdir("/");
	pages_free();
}

