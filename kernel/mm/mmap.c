#include "mmap.h"
#include "addrspace.h"
#include "vma.h"
#include "page.h"
#include "../proc/task.h"
#include "../proc/proc.h"
#include "../vfs/fdtable.h"
#include "../mem/heap.h"
#include "../arch/x86/paging_hw.h"

static int flags_to_vma(int flags) {
    int vf = 0;
    if (flags & MMAP_MAP_PRIVATE) vf |= VMA_MAP_PRIVATE;
    if (flags & MMAP_MAP_SHARED) vf |= VMA_MAP_SHARED;
    if (flags & MMAP_MAP_ANON) vf |= VMA_MAP_ANON;
    else vf |= VMA_MAP_FILE;
    return vf;
}

int mm_mmap_sys(void* addr, u32 len, int prot, int flags, int fd, u32 off) {
    struct task* t = task_current(); struct proc* p=t?t->owner:0;
    if (!p || !p->as || !len) return -1;
    if ((flags & MMAP_MAP_FIXED) != 0) return -1;
    u32 alen = (len + 4095U) & ~4095U;
    u32 start = addr ? ((u32)addr & ~4095U) : addrspace_find_gap(p->as, alen);
    struct vma* v = (struct vma*)kmalloc(sizeof(*v), 8); if (!v) return -1;
    v->start = start; v->end = start + alen;
    v->prot = (u32)prot; v->flags = (u32)flags_to_vma(flags); v->file = 0; v->file_off = off; v->next = 0;
    if (!(flags & MMAP_MAP_ANON)) {
        struct file* f = fdtable_get(&p->fdt, fd); if (!f || !f->vnode) { kfree(v); return -1; }
        v->file = f->vnode; vnode_ref(v->file);
    }
    if (vma_insert(p->as, v) < 0) { if(v->file)vnode_put(v->file); kfree(v); return -1; }
    return (int)start;
}

int mm_munmap_sys(void* addr, u32 len) {
    struct task* t = task_current(); struct proc* p=t?t->owner:0;
    if (!p || !p->as || !len) return -1;
    u32 start=((u32)addr)&~4095U, end=(start + len + 4095U)&~4095U;
    u32* pd = hw_pagedir_virt(p->cr3);
    for (u32 va=start; va<end; va+=4096U) {
        u32 pdei=hw_pde_index(va), ptei=hw_pte_index(va);
        if (!(pd[pdei] & PTE_PRESENT)) continue;
        u32* pt=hw_pt_virt_from_pde(pd[pdei]);
        u32 pte=pt[ptei]; if(!(pte&PTE_PRESENT)) continue;
        struct page* pg = page_for_phys(pte & PTE_ADDR_MASK);
        pt[ptei]=0; hw_invlpg(va); page_put(pg);
    }
    vma_remove_range(p->as,start,end);
    return 0;
}
