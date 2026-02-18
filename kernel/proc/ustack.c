#include "proc.h"
#include "../mem/pmm.h"
#include "../mem/vmm.h"

int proc_setup_user_stack(struct proc* p, u32* out_esp) {
    const u32 pages = 4;
    u32 base = USER_STACK_TOP - pages * 4096U;
    for (u32 i = 0; i < pages; ++i) {
        u32 fr = pmm_alloc_frame();
        if (!fr) return -1;
        vmm_map_page_in(p->cr3, base + i * 4096U, fr, PAGE_USER | PAGE_RW);
    }
    *out_esp = USER_STACK_TOP;
    return 0;
}
