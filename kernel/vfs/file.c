#include "file.h"
#include "vnode.h"
#include "../mem/heap.h"
#include "../mem/mem.h"

struct file* file_create(struct vnode* vnode, u32 flags) {
    struct file* f = (struct file*)kmalloc(sizeof(struct file), 8);
    if (!f) return 0;
    mem_set(f, 0, sizeof(*f));
    f->refs = 1;
    f->flags = flags;
    f->vnode = vnode;
    vnode_ref(vnode);
    return f;
}

struct file* file_create_special(const struct file_ops* ops, void* priv, u32 flags) {
    struct file* f = (struct file*)kmalloc(sizeof(struct file), 8);
    if (!f) return 0;
    mem_set(f, 0, sizeof(*f));
    f->refs = 1;
    f->flags = flags;
    f->ops = ops;
    f->priv = priv;
    return f;
}

void file_ref(struct file* f) { if (f) f->refs++; }
void file_put(struct file* f) {
    if (!f || !f->refs) return;
    f->refs--;
    if (f->refs == 0) {
        if (f->ops && f->ops->close) f->ops->close(f);
        if (f->vnode) vnode_put(f->vnode);
        kfree(f);
    }
}
