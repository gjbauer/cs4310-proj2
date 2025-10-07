// based on cs3650 starter code

#ifndef BITMAP_H
#define BITMAP_H
#include <stdio.h>
#include <stdint.h>

int bitmap_get(void* bm, int ii);
void bitmap_put(void* bm, int ii, int vv);
void bitmap_print(void* bm, int size);

#endif
