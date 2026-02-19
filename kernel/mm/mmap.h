#ifndef POOS_MM_MMAP_H
#define POOS_MM_MMAP_H

#include "../types.h"

#define MMAP_PROT_READ  0x1
#define MMAP_PROT_WRITE 0x2
#define MMAP_PROT_EXEC  0x4

#define MMAP_MAP_PRIVATE 0x01
#define MMAP_MAP_SHARED  0x02
#define MMAP_MAP_FIXED   0x10
#define MMAP_MAP_ANON    0x20

int mm_mmap_sys(void* addr, u32 len, int prot, int flags, int fd, u32 off);
int mm_munmap_sys(void* addr, u32 len);

#endif
