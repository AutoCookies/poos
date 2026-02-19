#include "proc.h"
#include "../cgroup/cgroup.h"
int proc_attach_cgroup(struct proc* p, struct cgroup* cg){ if(!p||!cg) return -1; if(cgroup_join(cg,p)<0) return -1; if(p->cgrp) cgroup_leave(p->cgrp); p->cgrp=cg; cgroup_get(cg); return 0; }
