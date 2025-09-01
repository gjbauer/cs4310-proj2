#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "libmkfs.h"
#include "slist.h"
#include "nufs.h"

int
main(int argc, char *argv[])
{
	char buf[256];
	storage_init("data.nufs");
	
	tree_lookup("/");
	
	//tree_lookup("/dir/dir");
	
	//readdir("/");	// Empty
	nufs_mknod("/hello.txt", 755, 0);
	nufs_write("/hello.txt", "hello!", 6, 0, 0);
	//write("/hello.txt", "hello!", 6, 6);
	//write("/hello.txt", "hello!", 6, 12);
	nufs_read("/hello.txt", buf, 6, 0, 0);	// < focus here...
	printf("%s\n", buf);
	//write("/hello.txt", "hello!", 6, 18);
	//read("/hello.txt", buf, 18, 6);	// < focus here...
	//printf("%s\n", buf);
	
	/*read("/hello.txt", buf, 24, 0);	// < focus here...
	printf("%s\n", buf);*/
	
	//readdir("/");
	
	nufs_mkdir("/dir", 755);
	nufs_mkdir("/dir/dir", 755);
	
	//print_directory(get_inode(tree_lookup("/")));
	
	nufs_readdir("/", 0, 0, 0, 0);
	
	//read("/hello.txt", buf, 5000, 4096);
	
	
	
	//readdir("/");
	
	//mknod("/dir", 755);
	
	//readdir("/");
	
	/*write("/hello.txt", "hello!", 6, 0);*/
	//readdir("/");
	//read("/hello.txt", buf, 24, 0);
	//printf("%s\n", buf);	// hello!
	
	
	
	//mknod("/dir/newmsg.txt", 755);
	//write("/dir/newmsg.txt", "newmsg!", 6, 0);
	//read("/dir/newmsg.txt", buf, 0, 0);
	//printf("reading root\n");
	//readdir("/");
	//printf("reading /dir\n");
	//readdir("/dir");
	//read("/hello.txt", buf, 0, 0);
	//printf("%s\n", buf);	// hello!
	//printf("%s\n", buf);	// newmsg!
	//mknod("/dir/two.txt", 755);
	//write("/dir/two.txt", "two!", 6, 0);
	//read("/dir/two.txt", buf, 0, 0);
	//printf("%s\n", buf);	// two!
	//readdir("/dir");
	//mkdir("/dir/dir", 755);
	//mknod("/dir/dir/one.txt", 755);
	//printf("reading /dir/dir\n");
	//readdir("/dir/dir");
	//write("/dir/dir/one.txt", "one!", 6, 0);
	//printf("reading /dir/dir/one.txt\n");
	//read("/dir/dir/one.txt", buf, 0, 0);
	//printf("%s\n", buf);
	//printf("reading /dir\n");
	//readdir("/dir");
	//read("/hello.txt", buf, 0, 0);
	//printf("%s\n", buf);	// hello!
	pages_free();
}
