#include "mm.h"
#include "addrspace.h"
#include "vma.h"
#include "cow.h"
#include "page.h"
#include "pagecache.h"
#include "../proc/proc.h"
#include "../proc/task.h"
#include "../mem/mem.h"
#include "../arch/x86/paging_hw.h"

struct page* anon_alloc_zero(void);
struct page* filemap_get_page(struct vnode* vn, u32 file_off);

static u32 vma_to_pte_flags(struct vma* v) {
    u32 f = PTE_USER;
    if (v->prot & VMA_PROT_WRITE) f |= PTE_RW;
    return f;
}

int mm_handle_page_fault(struct trapframe* tf, u32 fault_addr) {
    struct vm_counters* c = mm_counters();
    c->faults_total++;
    struct task* t = task_current();
    struct proc* p = t ? t->owner : 0;
    if (!p || !p->as) return -1;
    u32 va = fault_addr & ~0xFFFU;
    struct vma* v = addrspace_find_vma(p->as, fault_addr);
    if (!v) return -1;

    bool present = (tf->err_code & 1U) != 0;
    bool wr = (tf->err_code & 2U) != 0;
    if (present && wr) {
        if (cow_handle_write_fault(p, fault_addr) == 0) { c->faults_handled++; return 0; }
        return -1;
    }
    if (present) return -1;

    struct page* pg = 0;
    if (v->flags & VMA_MAP_ANON) pg = anon_alloc_zero();
    else if ((v->flags & VMA_MAP_FILE) && v->file) {
        u32 page_off = (fault_addr - v->start) & ~0xFFFU;
        pg = filemap_get_page(v->file, v->file_off + page_off);
    }
    if (!pg) return -1;

    u32* pd = hw_pagedir_virt(p->cr3);
    u32 pdei = hw_pde_index(va), ptei = hw_pte_index(va);
    if (!(pd[pdei] & PTE_PRESENT)) {
        struct page* ptp = page_alloc(0);
        if (!ptp) { page_put(pg); return -1; }
        mem_set((void*)(ptp->phys + KERNEL_BASE),0,4096);
        pd[pdei] = ptp->phys | PTE_PRESENT | PTE_USER | PTE_RW;
    }
    u32* pt = hw_pt_virt_from_pde(pd[pdei]);
    pt[ptei] = pg->phys | PTE_PRESENT | vma_to_pte_flags(v);
    if (v->flags & VMA_MAP_PRIVATE) pg->flags |= PAGEF_COW;
    hw_invlpg(va);
    mm_counters()->anon_pages_alloc += (v->flags & VMA_MAP_ANON) ? 1U : 0U;
    c->faults_handled++;
    return 0;
}
