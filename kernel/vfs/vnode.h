#ifndef POOS_VFS_VNODE_H
#define POOS_VFS_VNODE_H

#include "../types.h"

enum vnode_type { VNODE_REG = 1, VNODE_DIR = 2, VNODE_DEV = 3 };

struct vstat {
    u32 mode;
    u32 size;
    u32 type;
};

struct vdirent {
    u32 ino;
    u32 type;
    char name[32];
};

struct vnode;

struct vnode_ops {
    int (*lookup)(struct vnode* dir, const char* name, struct vnode** out);
    int (*read)(struct vnode* vn, u32 off, void* buf, u32 len);
    int (*write)(struct vnode* vn, u32 off, const void* buf, u32 len);
    int (*readdir)(struct vnode* vn, u32* cookie, struct vdirent* out);
    int (*getattr)(struct vnode* vn, struct vstat* out);
};

struct vnode {
    u32 refs;
    enum vnode_type type;
    const struct vnode_ops* ops;
    void* data;
};

void vnode_init(struct vnode* vn, enum vnode_type type, const struct vnode_ops* ops, void* data);
void vnode_ref(struct vnode* vn);
void vnode_put(struct vnode* vn);

#endif
