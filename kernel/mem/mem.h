#ifndef POOS_MEM_MEM_H
#define POOS_MEM_MEM_H

#include "../types.h"
#include "../bootinfo.h"

void* mem_set(void* dest, u8 value, usize count);
void* mem_copy(void* dest, const void* src, usize count);
int mem_cmp(const void* a, const void* b, usize count);

void mem_init(struct BootInfo* bootinfo);
void mem_print_summary(void);
void mem_sanity_check(void);

#endif
