#include <math.h>

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
