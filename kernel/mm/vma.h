#ifndef POOS_MM_VMA_H
#define POOS_MM_VMA_H

#include "../types.h"
#include "../vfs/vnode.h"

#define VMA_PROT_READ  0x1
#define VMA_PROT_WRITE 0x2
#define VMA_PROT_EXEC  0x4

#define VMA_MAP_PRIVATE 0x01
#define VMA_MAP_SHARED  0x02
#define VMA_MAP_ANON    0x04
#define VMA_MAP_FILE    0x08

struct vma {
    u32 start;
    u32 end;
    u32 prot;
    u32 flags;
    struct vnode* file;
    u32 file_off;
    struct vma* next;
};

struct addrspace;
struct vma* vma_find(struct addrspace* as, u32 addr);
int vma_insert(struct addrspace* as, struct vma* v);
void vma_remove_range(struct addrspace* as, u32 start, u32 end);
struct vma* vma_clone_list(struct vma* src);
void vma_free_list(struct vma* v);

#endif
