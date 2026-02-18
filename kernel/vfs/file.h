#ifndef POOS_VFS_FILE_H
#define POOS_VFS_FILE_H

#include "vnode.h"

struct file {
    u32 refs;
    u32 pos;
    u32 flags;
    struct vnode* vnode;
};

struct file* file_create(struct vnode* vnode, u32 flags);
void file_ref(struct file* f);
void file_put(struct file* f);

#endif
