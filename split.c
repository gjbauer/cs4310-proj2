#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "directory.h"

const char *path0 = "/path";
const char *path1 = "/path/one/";

char *split(const char *path, int n) {
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

int main() {
	printf("%s\n", split(path0, 0));
	printf("%s\n", split(path0, 1));
	printf("%s\n", split(path1, 2));
}
