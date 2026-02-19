#ifndef POOS_MM_ADDRSPACE_H
#define POOS_MM_ADDRSPACE_H

#include "../types.h"
#include "vma.h"

#define MMAP_BASE 0x40000000U

struct addrspace {
    u32 cr3;
    struct vma* vmas;
    u32 mmap_base;
};

void addrspace_init(struct addrspace* as, u32 cr3);
struct vma* addrspace_find_vma(struct addrspace* as, u32 addr);
u32 addrspace_find_gap(struct addrspace* as, u32 len);

#endif
