#ifndef POOS_VFS_FILE_H
#define POOS_VFS_FILE_H

#include "vnode.h"

struct file;

struct file_ops {
    int (*read)(struct file* f, u32 off, void* buf, u32 len);
    int (*write)(struct file* f, u32 off, const void* buf, u32 len);
    int (*close)(struct file* f);
};

struct file {
    u32 refs;
    u32 pos;
    u32 flags;
    struct vnode* vnode;
    const struct file_ops* ops;
    void* priv;
};

struct file* file_create(struct vnode* vnode, u32 flags);
struct file* file_create_special(const struct file_ops* ops, void* priv, u32 flags);
void file_ref(struct file* f);
void file_put(struct file* f);

#endif
