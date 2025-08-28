#include "bitmap.h"

int bitmap_get(void* bm, int ii) {
	int* ptr = (int*)bm;
	int os = ii / sizeof(int);
	ptr =  ptr + os;
	return *ptr;
}

void bitmap_put(void* bm, int ii, int vv) {
	int* ptr = (int*)bm;
	int os = ii / sizeof(int);
	ptr =  ptr + os;
	*ptr = vv;
}

void bitmap_print(void* bm, int size) {
	return;
}

