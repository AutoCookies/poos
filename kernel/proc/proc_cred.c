#include "proc.h"
#include "task.h"
#include "../sec/cred.h"
#include "../sec/caps.h"
#include "../vfs/mount.h"
#include "../vfs/vfs.h"
#include "../vfs/path.h"
#include "../vfs/vnode.h"
#include "../sec/audit.h"
#include "../ns/ns_proxy.h"

struct cred* cred_current(void){ struct task* t=task_current(); return (t&&t->owner)?t->owner->cred:0; }
struct vnode* proc_current_root(void){ struct task* t=task_current(); if(!t||!t->owner) return mount_root(); if(t->owner->nsproxy && t->owner->nsproxy->mnt && t->owner->nsproxy->mnt->root) return t->owner->nsproxy->mnt->root; if(!t->owner->root_vnode) return mount_root(); return t->owner->root_vnode; }

int proc_setuid(u32 uid){ struct cred* c=cred_current(); if(!c) return -1; if(c->euid!=0 && uid!=c->uid && uid!=c->suid) return -1; if(c->euid==0){ c->uid=uid; c->suid=uid; } c->euid=uid; return 0; }
int proc_setgid(u32 gid){ struct cred* c=cred_current(); if(!c) return -1; if(c->euid!=0 && gid!=c->gid && gid!=c->sgid) return -1; if(c->euid==0){ c->gid=gid; c->sgid=gid; } c->egid=gid; return 0; }
int proc_capset(u32 pid, u32 caps){ struct proc* p=proc_find(pid); struct cred* c=cred_current(); if(!p||!c||!cred_has_cap(c,CAP_SYS_ADMIN)) return -1; p->cred->cap_permitted=caps; p->cred->cap_effective=caps; audit_log("capset"); return 0; }
int proc_chroot(const char* path){ struct cred* c=cred_current(); if(!c||!cred_has_cap(c,CAP_SYS_ADMIN)) return -1; struct vnode* vn=0; if(vfs_resolve(path,&vn)<0) return -1; struct task* t=task_current(); if(!t||!t->owner){ vnode_put(vn); return -1; } if(t->owner->nsproxy && t->owner->nsproxy->mnt){ if(t->owner->nsproxy->mnt->root) vnode_put(t->owner->nsproxy->mnt->root); t->owner->nsproxy->mnt->root=vn; } if(t->owner->root_vnode) vnode_put(t->owner->root_vnode); t->owner->root_vnode=vn; vnode_ref(vn); audit_log("chroot"); return 0; }
