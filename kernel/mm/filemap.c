#include "pagecache.h"
#include "mm.h"

struct page* filemap_get_page(struct vnode* vn, u32 file_off) {
    bool hit = false;
    struct page* p = pagecache_get(vn, file_off / 4096U, &hit);
    if (p && !hit) mm_counters()->file_pages_loaded++;
    return p;
}
