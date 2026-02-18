#ifndef POOS_FS_TARFS_H
#define POOS_FS_TARFS_H

#include "../vfs/vnode.h"

int tarfs_mount_root(const u8* tar, u32 size, struct vnode** out_root);

#endif
