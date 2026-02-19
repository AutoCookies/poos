#ifndef POOS_MM_PAGECACHE_H
#define POOS_MM_PAGECACHE_H

#include "../types.h"
#include "../vfs/vnode.h"
#include "page.h"

void pagecache_init(void);
struct page* pagecache_get(struct vnode* vn, u32 page_index, bool* hit);
void pagecache_put(struct page* p);
void pagecache_stats(u32* hits, u32* misses, u32* entries);

#endif
