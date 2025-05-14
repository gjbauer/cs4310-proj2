#include "bitmap.h"

int bitmap_get(void* bm, int ii) {
	int* ptr = (int*)bm;
	ptr =  ptr + ii;
	return *ptr;
}

void bitmap_put(void* bm, int ii, int vv) {
	int* ptr = (int*)bm;
	ptr =  ptr + ii;
	*ptr = vv;
}

void bitmap_print(void* bm, int size) {
	return;
}

