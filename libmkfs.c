#include <string.h>
#include <stdint.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "nufs.h"
#include "mkfs.h"
#include <stdlib.h>

void
mkfs() {
	pages_init("data.nufs");
	nufs_mkdir("/", 755);
	nufs_mknod("/one.txt", 755, 0);
	nufs_mknod("/two.txt", 755, 0);
	pages_free();
}

