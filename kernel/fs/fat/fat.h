#ifndef POOS_FAT_H
#define POOS_FAT_H
#include "../../types.h"
#include "../../blk/blkdev.h"
#include "../../vfs/vnode.h"

struct fat_fs;
int fat_mount(struct blkdev* dev, struct vnode** out_root);
int fat_sync(void);

#endif
