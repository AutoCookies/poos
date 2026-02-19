#include "page.h"
#include "../mem/pmm.h"
#include "../mem/mem.h"

#define PAGE_MAX (1024U*1024U)
static struct page g_pages[PAGE_MAX];
static bool g_inited;

void page_init(void) {
    if (g_inited) return;
    mem_set(g_pages, 0, sizeof(g_pages));
    for (u32 i=0;i<PAGE_MAX;++i) g_pages[i].phys = i*4096U;
    g_inited = true;
}

struct page* page_for_phys(u32 phys) {
    u32 idx = phys / 4096U;
    if (idx >= PAGE_MAX) return 0;
    return &g_pages[idx];
}

struct page* page_alloc(u32 flags) {
    u32 phys = pmm_alloc_frame();
    if (!phys) return 0;
    struct page* p = page_for_phys(phys);
    if (!p) return 0;
    p->refcount = 1;
    p->flags = flags;
    p->owner = 0;
    return p;
}

void page_get(struct page* p) { if (p) ++p->refcount; }

void page_put(struct page* p) {
    if (!p) return;
    POOS_ASSERT(p->refcount > 0);
    --p->refcount;
    if (p->refcount == 0) {
        p->flags = 0;
        p->owner = 0;
        pmm_free_frame(p->phys);
    }
}
