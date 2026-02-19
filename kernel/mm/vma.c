#include "vma.h"
#include "addrspace.h"
#include "../mem/heap.h"
#include "../mem/mem.h"

static bool compat(struct vma* a, struct vma* b) {
    return a->end == b->start && a->prot == b->prot && a->flags == b->flags && a->file == b->file && (a->file_off + (a->end-a->start) == b->file_off);
}

struct vma* vma_find(struct addrspace* as, u32 addr) {
    for (struct vma* it=as->vmas; it; it=it->next) if (addr >= it->start && addr < it->end) return it;
    return 0;
}

int vma_insert(struct addrspace* as, struct vma* v) {
    struct vma** pp = &as->vmas;
    while (*pp && (*pp)->start < v->start) pp = &(*pp)->next;
    if (*pp && v->end > (*pp)->start) return -1;
    if (pp != &as->vmas) {
        struct vma* prev = as->vmas;
        while (prev && prev->next != *pp) prev = prev->next;
        if (prev && prev->end > v->start) return -1;
    }
    v->next = *pp; *pp = v;
    if (v->next && compat(v, v->next)) { struct vma* n=v->next; v->end=n->end; v->next=n->next; kfree(n); }
    return 0;
}

void vma_remove_range(struct addrspace* as, u32 start, u32 end) {
    struct vma** pp=&as->vmas;
    while (*pp) {
        struct vma* v=*pp;
        if (end <= v->start) break;
        if (start >= v->end) { pp=&v->next; continue; }
        if (start <= v->start && end >= v->end) { *pp=v->next; kfree(v); continue; }
        if (start <= v->start) { v->start=end; v->file_off += (end-start); break; }
        if (end >= v->end) { v->end=start; pp=&v->next; continue; }
        struct vma* right=(struct vma*)kmalloc(sizeof(*right),8); if(!right) return;
        *right=*v; right->start=end; right->file_off += (end - v->start); right->next=v->next;
        v->end=start; v->next=right; break;
    }
}

struct vma* vma_clone_list(struct vma* src) {
    struct vma* head=0; struct vma** tail=&head;
    for (; src; src=src->next) {
        struct vma* n=(struct vma*)kmalloc(sizeof(*n),8); if(!n){vma_free_list(head);return 0;}
        *n=*src; n->next=0; *tail=n; tail=&n->next;
        if (n->file) vnode_ref(n->file);
    }
    return head;
}

void vma_free_list(struct vma* v) {
    while (v) { struct vma* n=v->next; if(v->file) vnode_put(v->file); kfree(v); v=n; }
}
