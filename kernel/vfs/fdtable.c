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

void fdtable_clone(struct fdtable* dst, struct fdtable* src) {
    fdtable_init(dst);
    for (u32 i = 0; i < FD_MAX; ++i) {
        if (src->files[i]) {
            dst->files[i] = src->files[i];
            file_ref(dst->files[i]);
        }
    }
}
