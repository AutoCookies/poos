#ifndef POOS_VFS_MOUNT_H
#define POOS_VFS_MOUNT_H

#include "vnode.h"

struct mount {
    const char* path;
    struct vnode* root;
};

void mount_init(void);
int mount_attach(const char* path, struct vnode* root);
struct vnode* mount_root(void);
struct vnode* mount_find(const char* path);

#endif
