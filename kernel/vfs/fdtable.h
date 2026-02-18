#ifndef POOS_VFS_FDTABLE_H
#define POOS_VFS_FDTABLE_H

#include "file.h"

#define FD_MAX 64U

struct fdtable {
    struct file* files[FD_MAX];
};

void fdtable_init(struct fdtable* fdt);
int fdtable_alloc(struct fdtable* fdt, struct file* f);
struct file* fdtable_get(struct fdtable* fdt, int fd);
int fdtable_close(struct fdtable* fdt, int fd);
void fdtable_clone(struct fdtable* dst, struct fdtable* src);

#endif
