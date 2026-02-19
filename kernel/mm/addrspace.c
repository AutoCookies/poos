#include "addrspace.h"

void addrspace_init(struct addrspace* as, u32 cr3) {
    as->cr3 = cr3;
    as->vmas = 0;
    as->mmap_base = MMAP_BASE;
}

struct vma* addrspace_find_vma(struct addrspace* as, u32 addr) { return vma_find(as, addr); }

u32 addrspace_find_gap(struct addrspace* as, u32 len) {
    u32 need = (len + 4095U) & ~4095U;
    u32 at = as->mmap_base;
    for (struct vma* it = as->vmas; it; it = it->next) {
        if (at + need <= it->start) return at;
        if (it->end > at) at = it->end;
    }
    return at;
}
