#ifndef POOS_VFS_H
#define POOS_VFS_H

#include "vnode.h"
#include "file.h"

void vfs_init(void);
int vfs_mount(const char* path, struct vnode* root);
int vfs_open(const char* path, u32 flags, struct file** out);
int vfs_stat(const char* path, struct vstat* st);
int vfs_mkdir(const char* path, u32 mode);
int vfs_unlink(const char* path);
int vfs_rename(const char* oldp, const char* newp);
int vfs_sync(void);

#endif
