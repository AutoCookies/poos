#include "cow.h"
#include "page.h"
#include "mm.h"
#include "../proc/proc.h"
#include "../mem/mem.h"
#include "../arch/x86/paging_hw.h"

static u32 pte_user_flags(u32 pte) {
    return (pte & (PTE_USER|PTE_PRESENT));
}

int cow_fork_clone(struct proc* child, struct proc* parent) {
    u32* ppd = hw_pagedir_virt(parent->cr3);
    u32* cpd = hw_pagedir_virt(child->cr3);
    for (u32 pdei=0; pdei<768; ++pdei) {
        if (!(ppd[pdei] & PTE_PRESENT)) continue;
        struct page* cptp = page_alloc(0); if (!cptp) return -1;
        mem_set((void*)(cptp->phys + KERNEL_VIRT_BASE), 0, 4096);
        cpd[pdei] = cptp->phys | PTE_PRESENT | PTE_USER | PTE_RW;
        u32* ppt = hw_pt_virt_from_pde(ppd[pdei]);
        u32* cpt = (u32*)(cptp->phys + KERNEL_VIRT_BASE);
        for (u32 ptei=0; ptei<1024; ++ptei) {
            u32 pte = ppt[ptei];
            if (!(pte & PTE_PRESENT) || !(pte & PTE_USER)) continue;
            u32 phys = pte & PTE_ADDR_MASK;
            struct page* pg = page_for_phys(phys);
            if (!pg) return -1;
            page_get(pg);
            u32 np = (pte & ~(PTE_RW)) | PTE_COW;
            ppt[ptei] = np;
            cpt[ptei] = np;
        }
    }
    return 0;
}

int cow_handle_write_fault(struct proc* p, u32 fault_addr) {
    u32 va = fault_addr & ~0xFFFU;
    u32* pd = hw_pagedir_virt(p->cr3);
    u32 pdei=hw_pde_index(va), ptei=hw_pte_index(va);
    if (!(pd[pdei] & PTE_PRESENT)) return -1;
    u32* pt = hw_pt_virt_from_pde(pd[pdei]);
    u32 pte = pt[ptei];
    if (!(pte & PTE_PRESENT) || !(pte & PTE_COW)) return -1;
    mm_counters()->cow_faults++;
    u32 phys = pte & PTE_ADDR_MASK;
    struct page* oldp = page_for_phys(phys);
    if (!oldp) return -1;
    if (oldp->refcount == 1) {
        pt[ptei] = (pte | PTE_RW) & ~PTE_COW;
        hw_invlpg(va);
        return 0;
    }
    struct page* np = page_alloc(0);
    if (!np) return -1;
    mem_copy((void*)(np->phys + KERNEL_VIRT_BASE), (void*)(phys + KERNEL_VIRT_BASE), 4096);
    pt[ptei] = (np->phys | pte_user_flags(pte) | PTE_RW) & ~PTE_COW;
    page_put(oldp);
    mm_counters()->cow_copies++;
    hw_invlpg(va);
    return 0;
}
