#include "string.h"
#include "directory.h"
#include <string.h>
#include <stdlib.h>

char *split(const char *path, int n)
{
	int rv=0;
	char splt[DIR_NAME];
	if (n==0) {
		strcpy(splt, "/");
	} else {
		int c=0, i=0;
		for (; path[i] && c<n+1; i++) {
			splt[i]=path[i];
			if (path[i]=='/') c++;
		}
		if (splt[i-1]=='/') splt[i-1]='\0';
	}
	char *buf = (char*)calloc(DIR_NAME, sizeof(char));
	strncpy(buf, splt, DIR_NAME);
	return buf;
}

int
count_l(const char *path)
{
	int c=0;
	for(int i=0; path[i]; i++) {
		if (path[i]=='/') c++;
	}
	return c;
}
