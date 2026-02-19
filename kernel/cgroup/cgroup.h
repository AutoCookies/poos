#ifndef POOS_CGROUP_H
#define POOS_CGROUP_H
#include "../types.h"
struct proc;
struct cgroup { u32 id; u32 refcnt; u32 pids_max; u32 pids_cur; u32 mem_max; u32 mem_cur; u32 cpu_pct; u32 cpu_ticks; u32 cpu_throttles; u32 oom_kills; };
void cgroup_init(void);
struct cgroup* cgroup_root(void);
struct cgroup* cgroup_create(void);
void cgroup_get(struct cgroup* cg);
void cgroup_put(struct cgroup* cg);
int cgroup_join(struct cgroup* cg, struct proc* p);
void cgroup_leave(struct cgroup* cg);
int cgroup_set_limits(struct cgroup* cg, u32 mem, u32 pids, u32 cpu);
int cgroup_can_fork(struct cgroup* cg);
#endif
