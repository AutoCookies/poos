#ifndef POOS_VFS_H
#define POOS_VFS_H

#include "vnode.h"
#include "file.h"

void vfs_init(void);
int vfs_mount(const char* path, struct vnode* root);
int vfs_open(const char* path, u32 flags, struct file** out);
int vfs_stat(const char* path, struct vstat* st);

#endif
