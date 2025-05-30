#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include "pages.h"
#include "inode.h"
#include "bitmap.h"
#include "directory.h"
#include "mkfs.h"
#include "mfs.h"

int
main(int argc, char *argv[])
{
	char buf[256];
	storage_init("data.nufs");
	
	
	//readdir("/");	// Empty
	//mknod("/hello.txt", 755);
	//write("/hello.txt", "hello!", 6, 0);
	//write("/hello.txt", "hello!", 6, 6);
	//write("/hello.txt", "hello!", 6, 12);
	//read("/hello.txt", buf, 12, 6);	// < focus here...
	//printf("%s\n", buf);
	//write("/hello.txt", "hello!", 6, 18);
	//read("/hello.txt", buf, 18, 6);	// < focus here...
	//printf("%s\n", buf);
	
	/*read("/hello.txt", buf, 24, 0);	// < focus here...
	printf("%s\n", buf);*/
	
	//readdir("/");
	
	//mknod("/dir", 755);
	
	readdir("/");
	
	mknod("/dir", 755);
	
	
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
