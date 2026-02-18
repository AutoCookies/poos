#include "fdtable.h"
#include "../mem/mem.h"

void fdtable_init(struct fdtable* fdt) { mem_set(fdt, 0, sizeof(*fdt)); }

int fdtable_alloc(struct fdtable* fdt, struct file* f) {
    for (u32 i = 0; i < FD_MAX; ++i) {
        if (!fdt->files[i]) {
            fdt->files[i] = f;
            file_ref(f);
            return (int)i;
        }
    }
    return -1;
}

int fdtable_set(struct fdtable* fdt, int fd, struct file* f) {
    if (!fdt || fd < 0 || (u32)fd >= FD_MAX || !f) return -1;
    if (fdt->files[fd]) file_put(fdt->files[fd]);
    fdt->files[fd] = f;
    file_ref(f);
    return fd;
}

struct file* fdtable_get(struct fdtable* fdt, int fd) {
    if (!fdt || fd < 0 || (u32)fd >= FD_MAX) return 0;
    return fdt->files[fd];
}

int fdtable_close(struct fdtable* fdt, int fd) {
    if (!fdt || fd < 0 || (u32)fd >= FD_MAX || !fdt->files[fd]) return -1;
    file_put(fdt->files[fd]);
    fdt->files[fd] = 0;
    return 0;
}

int fdtable_dup2(struct fdtable* fdt, int oldfd, int newfd) {
    if (!fdt || oldfd < 0 || newfd < 0 || (u32)oldfd >= FD_MAX || (u32)newfd >= FD_MAX) return -1;
    struct file* f = fdt->files[oldfd];
    if (!f) return -1;
    if (oldfd == newfd) return newfd;
    if (fdt->files[newfd]) {
        file_put(fdt->files[newfd]);
        fdt->files[newfd] = 0;
    }
    fdt->files[newfd] = f;
    file_ref(f);
    return newfd;
}

void fdtable_clone(struct fdtable* dst, struct fdtable* src) {
    fdtable_init(dst);
    for (u32 i = 0; i < FD_MAX; ++i) {
        if (src->files[i]) {
            dst->files[i] = src->files[i];
            file_ref(dst->files[i]);
        }
    }
}
