#include "page.h"
#include "../mem/mem.h"

struct page* anon_alloc_zero(void) {
    struct page* p = page_alloc(0);
    if (!p) return 0;
    mem_set((void*)(p->phys + KERNEL_BASE), 0, 4096);
    return p;
}
