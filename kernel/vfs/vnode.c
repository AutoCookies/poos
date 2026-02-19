#include "vnode.h"

void vnode_init(struct vnode* vn, enum vnode_type type, const struct vnode_ops* ops, void* data) {
    vn->refs = 1;
    vn->type = type;
    vn->uid = 0;
    vn->gid = 0;
    vn->mode = (type==VNODE_DIR)?0755:0644;
    if(type==VNODE_DEV) vn->mode=0600;
    vn->ops = ops;
    vn->data = data;
}

void vnode_ref(struct vnode* vn) { if (vn) vn->refs++; }
void vnode_put(struct vnode* vn) { if (vn && vn->refs) vn->refs--; }
