#ifndef POOS_BLKDEV_H
#define POOS_BLKDEV_H
#include "../types.h"

struct blkdev;
typedef int (*blk_rw_fn)(struct blkdev* dev, u32 lba, u32 count, void* buf);

struct blkdev {
    const char* name;
    u32 sector_size;
    u32 total_sectors;
    blk_rw_fn read;
    blk_rw_fn write;
    void* priv;
};

int blkdev_register(struct blkdev* dev);
struct blkdev* blkdev_get(const char* name);
int blk_read(struct blkdev* dev, u32 lba, u32 count, void* buf);
int blk_write(struct blkdev* dev, u32 lba, u32 count, const void* buf);

#endif
