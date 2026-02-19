#ifndef POOS_MM_PAGE_H
#define POOS_MM_PAGE_H

#include "../types.h"

enum { PAGEF_COW=1U<<0, PAGEF_DIRTY=1U<<1, PAGEF_PINNED=1U<<2, PAGEF_CACHE=1U<<3 };

struct page {
    u32 phys;
    int refcount;
    u32 flags;
    void* owner;
};

void page_init(void);
struct page* page_for_phys(u32 phys);
struct page* page_alloc(u32 flags);
void page_get(struct page* p);
void page_put(struct page* p);

#endif
