#include "hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned int hash(const unsigned char *str)
{
    unsigned long hash = 137;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % 512;
}

char* extend(const char *path) {
	char c = (char)hash(path);
	char *hpath = calloc((strlen(path)+1), sizeof(char));
	strcpy(hpath, path);
	strcat(hpath, &c);
	return hpath;
}

/*int main() {
	printf("%ld\n", hash("hello") % 512);
}*/

