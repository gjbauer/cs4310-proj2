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
	dirent d;
	strcpy(d.name, "/");
	d.inum = 0;
	d.parent = NULL;
	d.next = NULL;
	memcpy(get_root_start(), (char*)&d, sizeof(dirent));
	//write("/", (char*)&d, sizeof(d), 0);
	//readdir("/");
	//mknod("/two.txt", 755);
	//readdir("/");
	pages_free();
}

