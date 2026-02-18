#ifndef POOS_MEM_HEAP_H
#define POOS_MEM_HEAP_H

#include "../types.h"

#define KHEAP_BASE 0xC1000000U
#define KHEAP_INITIAL_SIZE (16U * 4096U)

void heap_init(void);
void* kmalloc(usize size, usize align);
void kfree(void* ptr);
void heap_smoke_test(void);

#endif
