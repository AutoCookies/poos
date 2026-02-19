#include "cgroup.h"
#include "../mem/heap.h"
#include "../sec/audit.h"
static struct cgroup* g_root; static u32 g_cgid=1;
void cgroup_init(void){ if(!g_root){ g_root=cgroup_create(); if(g_root) g_root->id=0; } }
struct cgroup* cgroup_root(void){ return g_root; }
struct cgroup* cgroup_create(void){ struct cgroup* cg=(struct cgroup*)kmalloc(sizeof(*cg),8); if(!cg) return 0; cg->id=g_cgid++; cg->refcnt=1; cg->pids_max=0; cg->mem_max=0; cg->cpu_pct=100; cg->pids_cur=cg->mem_cur=cg->cpu_ticks=cg->cpu_throttles=cg->oom_kills=0; audit_log("cgroup.create"); return cg; }
void cgroup_get(struct cgroup* cg){ if(cg) cg->refcnt++; }
void cgroup_put(struct cgroup* cg){ if(!cg) return; if(--cg->refcnt) return; kfree(cg); }
int cgroup_join(struct cgroup* cg, struct proc* p){ (void)p; if(!cg) return -1; if(cg->pids_max && cg->pids_cur>=cg->pids_max) return -1; cg->pids_cur++; return 0; }
void cgroup_leave(struct cgroup* cg){ if(cg&&cg->pids_cur) cg->pids_cur--; }
int cgroup_set_limits(struct cgroup* cg, u32 mem, u32 pids, u32 cpu){ if(!cg) return -1; cg->mem_max=mem; cg->pids_max=pids; cg->cpu_pct=cpu?cpu:100; return 0; }
int cgroup_can_fork(struct cgroup* cg){ if(!cg) return 1; return !(cg->pids_max && cg->pids_cur>=cg->pids_max); }
