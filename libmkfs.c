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
	mkdir("/", 755);
	mknod("/one.txt", 755);
	//mknod("/two.txt", 755);
	pages_free();
}

