#include "pagecache.h"
#include "../mem/heap.h"
#include "../mem/mem.h"

#define PCACHE_MAX 128U
struct pc_entry { struct vnode* vn; u32 idx; struct page* page; u32 age; };
static struct pc_entry g_pc[PCACHE_MAX];
static u32 g_tick, g_hits, g_misses;

void pagecache_init(void) { mem_set(g_pc,0,sizeof(g_pc)); g_tick=g_hits=g_misses=0; }

static void fill_page(struct vnode* vn, u32 idx, struct page* p) {
    void* kva = (void*)(p->phys + KERNEL_VIRT_BASE);
    mem_set(kva,0,4096);
    if (vn && vn->ops && vn->ops->read) {
        int rd = vn->ops->read(vn, idx*4096U, kva, 4096);
        (void)rd;
    }
}

struct page* pagecache_get(struct vnode* vn, u32 page_index, bool* hit) {
    ++g_tick;
    for (u32 i=0;i<PCACHE_MAX;++i) {
        if (g_pc[i].page && g_pc[i].vn==vn && g_pc[i].idx==page_index) {
            g_pc[i].age=g_tick; page_get(g_pc[i].page); ++g_hits; if(hit)*hit=true; return g_pc[i].page;
        }
    }
    ++g_misses; if(hit)*hit=false;
    u32 slot=PCACHE_MAX;
    for (u32 i=0;i<PCACHE_MAX;++i) if(!g_pc[i].page){slot=i;break;}
    if (slot==PCACHE_MAX) {
        u32 oldest=~0U;
        for (u32 i=0;i<PCACHE_MAX;++i) if (g_pc[i].page->refcount<=1 && g_pc[i].age<oldest){ oldest=g_pc[i].age; slot=i; }
        if (slot==PCACHE_MAX) return 0;
        vnode_put(g_pc[slot].vn);
        page_put(g_pc[slot].page);
    }
    struct page* p = page_alloc(PAGEF_CACHE);
    if (!p) return 0;
    fill_page(vn,page_index,p);
    vnode_ref(vn);
    g_pc[slot]=(struct pc_entry){ .vn=vn,.idx=page_index,.page=p,.age=g_tick };
    page_get(p);
    return p;
}

void pagecache_put(struct page* p) { page_put(p); }
void pagecache_stats(u32* hits,u32* misses,u32* entries) {
    if(hits)*hits=g_hits;
    if(misses)*misses=g_misses;
    if(entries){u32 e=0;for(u32 i=0;i<PCACHE_MAX;++i) if(g_pc[i].page) ++e; *entries=e;}
}
