#include "mntns.h"
#include "../mem/heap.h"
#include "../vfs/vnode.h"
#include "ns.h"
struct mnt_ns* mntns_create(struct vnode* root){ struct mnt_ns* ns=(struct mnt_ns*)kmalloc(sizeof(*ns),8); if(!ns) return 0; ns->id=ns_next_id(); ns->refcnt=1; ns->root=root; if(root) vnode_ref(root); return ns; }
void mntns_get(struct mnt_ns* ns){ if(ns) ns->refcnt++; }
void mntns_put(struct mnt_ns* ns){ if(!ns) return; if(--ns->refcnt) return; if(ns->root) vnode_put(ns->root); kfree(ns); }
