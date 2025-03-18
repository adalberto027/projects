#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>

int init_alloc();
int cleanup();
void *alloc(int size);
void dealloc(void *ptr);

#endif // ALLOC_H
