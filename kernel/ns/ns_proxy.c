#include "ns_proxy.h"
#include "../mem/heap.h"
#include "../vfs/mount.h"
struct nsproxy* nsproxy_create_host(struct vnode* root){ struct nsproxy* n=(struct nsproxy*)kmalloc(sizeof(*n),8); if(!n) return 0; n->refcnt=1; n->mnt=mntns_create(root?root:mount_root()); n->pid=pidns_create(); n->net=netns_create(1); n->uts=utsns_create("poos"); n->user=userns_create(0,0,1); return n; }
void nsproxy_get(struct nsproxy* n){ if(n) n->refcnt++; }
void nsproxy_put(struct nsproxy* n){ if(!n) return; if(--n->refcnt) return; mntns_put(n->mnt); pidns_put(n->pid); netns_put(n->net); utsns_put(n->uts); userns_put(n->user); kfree(n); }
struct nsproxy* nsproxy_clone(struct nsproxy* s,u32 f, struct vnode* root){ if(!s) return nsproxy_create_host(root); struct nsproxy* n=(struct nsproxy*)kmalloc(sizeof(*n),8); if(!n) return 0; n->refcnt=1;
 n->mnt=(f&CLONE_NEWNS)?mntns_create(root?root:s->mnt->root):(mntns_get(s->mnt),s->mnt);
 n->pid=(f&CLONE_NEWPID)?pidns_create():(pidns_get(s->pid),s->pid);
 n->net=(f&CLONE_NEWNET)?netns_create(0):(netns_get(s->net),s->net);
 n->uts=(f&CLONE_NEWUTS)?utsns_create(s->uts->hostname):(utsns_get(s->uts),s->uts);
 n->user=(f&CLONE_NEWUSER)?userns_create(0,1000,1):(userns_get(s->user),s->user);
 return n; }
