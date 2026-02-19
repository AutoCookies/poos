#ifndef POOS_VFS_PERM_H
#define POOS_VFS_PERM_H
#include "vnode.h"
int vfs_check_perm(struct vnode* vn, int want);
int vfs_check_dir_op(struct vnode* dir, int want_write);
enum { VFS_PERM_READ=1, VFS_PERM_WRITE=2, VFS_PERM_EXEC=4 };
#endif
