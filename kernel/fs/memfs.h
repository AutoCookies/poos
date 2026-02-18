#ifndef POOS_MEMFS_H
#define POOS_MEMFS_H
#include "../vfs/vnode.h"
struct vnode* memfs_root(void);
int memfs_open_flat(const char* path, u32 flags, struct vnode** out);
#endif
