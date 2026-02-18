#include "vfs.h"
#include "mount.h"
#include "path.h"

void vfs_init(void) { mount_init(); }
int vfs_mount(const char* path, struct vnode* root) { return mount_attach(path, root); }

int vfs_open(const char* path, u32 flags, struct file** out) {
    struct vnode* vn = 0;
    if (vfs_resolve(path, &vn) < 0) return -1;
    struct file* f = file_create(vn, flags);
    vnode_put(vn);
    if (!f) return -1;
    *out = f;
    return 0;
}

int vfs_stat(const char* path, struct vstat* st) {
    struct vnode* vn = 0;
    if (vfs_resolve(path, &vn) < 0) return -1;
    if (!vn->ops || !vn->ops->getattr) { vnode_put(vn); return -1; }
    int rc = vn->ops->getattr(vn, st);
    vnode_put(vn);
    return rc;
}
