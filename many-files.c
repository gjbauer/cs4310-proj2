#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include "directory.h"

char*
ltoa(int l, char *buf, int radix)
{
	int i;
	for (i=0; (l / 10) >= 10; i++);
	for (int h, j=0; i>0; i--, j++) {
		h = pow(10, i);
		buf[j] = (l / h) + 48;
	}
	return buf;
}

int
main()
{
	FILE fd;
	char fn[DIR_NAME] = "";
	for (int i=0; i<50; i++) {
		ltoa(i, fn, 10);
		strcat(fn, ".nums");
		fd = open(fn, O_WRONLY|O_CREAT,0755);
		memset(fn, 0, DIR_NAME);
	}
}
