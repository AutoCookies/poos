#include "cred.h"
#include "caps.h"
#include "../mem/heap.h"
#include "../mem/mem.h"
struct cred* cred_alloc_root(void){
    struct cred* c=(struct cred*)kmalloc(sizeof(*c),8); if(!c) return 0;
    mem_set(c,0,sizeof(*c)); c->refs=1; c->umask=022; c->cap_permitted=CAP_MASK_ALL; c->cap_effective=CAP_MASK_ALL; c->cap_inheritable=CAP_MASK_ALL; return c;
}
void cred_ref(struct cred* c){ if(c) c->refs++; }
void cred_put(struct cred* c){ if(!c||!c->refs) return; c->refs--; if(!c->refs) kfree(c); }
struct cred* cred_clone(const struct cred* in){ struct cred* c=(struct cred*)kmalloc(sizeof(*c),8); if(!c) return 0; mem_copy(c,in,sizeof(*c)); c->refs=1; return c; }
int cred_setuid(struct cred* c, u32 uid){ if(!c) return -1; c->uid=uid; c->euid=uid; c->suid=uid; return 0; }
int cred_setgid(struct cred* c, u32 gid){ if(!c) return -1; c->gid=gid; c->egid=gid; c->sgid=gid; return 0; }
int cred_in_group(const struct cred* c, u32 gid){ if(!c) return 0; if(c->gid==gid||c->egid==gid) return 1; for(u32 i=0;i<c->ngroups;i++) if(c->groups[i]==gid) return 1; return 0; }
int cred_has_cap(const struct cred* c, u32 cap){ if(!c) return 0; if(c->euid==0) return 1; return caps_has(c->cap_effective,cap); }
