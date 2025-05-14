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
	strncpy(d.name, "/", DIR_NAME);
	d.inum = 0;
	d.parent = NULL;
	d.next = NULL;
	write("/", (char*)&d, sizeof(d), 0);
	dirent e;
	read("/", (char*)&e, sizeof(e), 0);
	printf("%s\n", e.name);
	//readdir("/");
	//mknod("/two.txt", 755);
	//readdir("/");
	pages_free();
}

