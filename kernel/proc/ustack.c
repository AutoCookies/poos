#include "proc.h"
#include "../mem/pmm.h"
#include "../mem/vmm.h"
#include "../mm/vma.h"
#include "../mem/heap.h"

int proc_setup_user_stack(struct proc* p, u32* out_esp) {
    const u32 pages = 4;
    u32 base = USER_STACK_TOP - pages * 4096U;
    for (u32 i = 0; i < pages; ++i) {
        u32 fr = pmm_alloc_frame();
        if (!fr) return -1;
        vmm_map_page_in(p->cr3, base + i * 4096U, fr, PAGE_USER | PAGE_RW);
    }
    struct vma* v=(struct vma*)kmalloc(sizeof(*v),8);
    if(v){ v->start=base; v->end=USER_STACK_TOP; v->prot=VMA_PROT_READ|VMA_PROT_WRITE; v->flags=VMA_MAP_PRIVATE|VMA_MAP_ANON; v->file=0; v->file_off=0; v->next=0; vma_insert(p->as,v);}
    *out_esp = USER_STACK_TOP;
    return 0;
}
