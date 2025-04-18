#include <stdbool.h>
#include <stdio.h>
#include <string.h>
bool
isnum(const char *path)
{
	char n[4] = ".num";
	int l = strlen(path) - strlen(n);
	int i;
	for (i=0; i<l; i++);
	for (int j=0; j<4; j++, i++) if (path[i]!=n[j]) return false;
	return true;
}

int
main()
{
	printf("%s\n", isnum("path.num") ? "true" : "false");
}
