#ifndef POOS_VFS_PATH_H
#define POOS_VFS_PATH_H

#include "vnode.h"

int path_next_component(const char** path, char* out_name, u32 out_size);
int vfs_resolve(const char* path, struct vnode** out);

#endif
