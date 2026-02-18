#include "proc.h"
#include "task.h"
#include "elf32.h"
#include "../vfs/vfs.h"
#include "../mem/heap.h"

int proc_load_elf_from_path(struct proc* p, const char* path, u32* entry, u32* esp) {
    struct file* f = 0;
    struct vstat st;
    if (vfs_open(path, 0, &f) < 0) return -1;
    if (!f->vnode->ops || !f->vnode->ops->getattr || f->vnode->ops->getattr(f->vnode, &st) < 0) { file_put(f); return -1; }
    u8* image = (u8*)kmalloc(st.size, 8);
    if (!image) { file_put(f); return -1; }
    int rd = f->vnode->ops->read(f->vnode, 0, image, st.size);
    if (rd < 0 || (u32)rd != st.size) { kfree(image); file_put(f); return -1; }
    int rc = elf32_load_image(p, image, st.size, entry);
    kfree(image);
    if (rc < 0) { file_put(f); return -1; }
    if (proc_setup_user_stack(p, esp) < 0) { file_put(f); return -1; }
    file_put(f);
    return 0;
}

int proc_exec_path_current(const char* path) {
    struct task* t = task_current();
    if (!t || !t->owner) return -1;
    u32 entry = 0, esp = 0;
    if (proc_load_elf_from_path(t->owner, path, &entry, &esp) < 0) return -1;
    t->tf.eip = entry;
    t->tf.useresp = esp;
    return 0;
}
