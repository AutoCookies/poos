#include "bcache.h"
#include "../mem/mem.h"
#include "../mm/budget.h"

#define BCACHE_NBUF 256
static struct bcache_buf g_bufs[BCACHE_NBUF];
static u32 g_tick; static struct bcache_stats g_stats;

static int flush_one(struct bcache_buf* b){ if(!b->valid||!b->dirty) return 0; if(blk_write(b->dev,b->idx*8,8,b->data)<0) return -1; b->dirty=0; g_stats.writebacks++; return 0; }

void bcache_init(void){ mem_set(g_bufs,0,sizeof(g_bufs)); mem_set(&g_stats,0,sizeof(g_stats)); g_tick=1; mm_budget_set_used(MM_BUDGET_BUFFER_CACHE, sizeof(g_bufs)); }

struct bcache_buf* bcache_get(struct blkdev* dev, u32 idx){
    struct bcache_buf* freeb=0; struct bcache_buf* lru=0;
    for(u32 i=0;i<BCACHE_NBUF;i++){
        struct bcache_buf* b=&g_bufs[i];
        if(b->valid && b->dev==dev && b->idx==idx){ b->pins++; b->last_used=g_tick++; g_stats.hits++; return b; }
        if(!b->valid && !freeb) freeb=b;
        if(b->pins==0 && (!lru || b->last_used<lru->last_used)) lru=b;
    }
    g_stats.misses++;
    struct bcache_buf* b=freeb?freeb:lru; if(!b) return 0;
    if(b->valid){ if(flush_one(b)<0) return 0; g_stats.evictions++; }
    b->dev=dev; b->idx=idx; b->pins=1; b->valid=1; b->dirty=0; b->last_used=g_tick++;
    if(blk_read(dev,idx*8,8,b->data)<0){ b->valid=0; b->pins=0; return 0; }
    return b;
}
void bcache_put(struct bcache_buf* b){ if(b&&b->pins) b->pins--; }
void bcache_mark_dirty(struct bcache_buf* b){ if(b) b->dirty=1; }
int bcache_sync(struct blkdev* dev){ for(u32 i=0;i<BCACHE_NBUF;i++){ struct bcache_buf* b=&g_bufs[i]; if(!b->valid) continue; if(dev && b->dev!=dev) continue; if(flush_one(b)<0) return -1; } return 0; }
void bcache_get_stats(struct bcache_stats* st){ if(st) *st=g_stats; }
