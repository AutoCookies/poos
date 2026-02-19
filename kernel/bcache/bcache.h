#ifndef POOS_BCACHE_H
#define POOS_BCACHE_H
#include "../types.h"
#include "../blk/blkdev.h"

#define BCACHE_BLOCK_SIZE 4096
struct bcache_buf { struct blkdev* dev; u32 idx; u8 data[BCACHE_BLOCK_SIZE]; u8 valid,dirty; u16 pins; u32 last_used; };
struct bcache_stats { u32 hits, misses, evictions, writebacks; };

void bcache_init(void);
struct bcache_buf* bcache_get(struct blkdev* dev, u32 block_index);
void bcache_put(struct bcache_buf* b);
void bcache_mark_dirty(struct bcache_buf* b);
int bcache_sync(struct blkdev* dev);
void bcache_get_stats(struct bcache_stats* st);
#endif
