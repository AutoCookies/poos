#include "vnode.h"

void vnode_init(struct vnode* vn, enum vnode_type type, const struct vnode_ops* ops, void* data) {
    vn->refs = 1;
    vn->type = type;
    vn->ops = ops;
    vn->data = data;
}

void vnode_ref(struct vnode* vn) { if (vn) vn->refs++; }
void vnode_put(struct vnode* vn) { if (vn && vn->refs) vn->refs--; }
