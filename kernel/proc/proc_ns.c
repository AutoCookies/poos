#include "proc.h"
#include "task.h"
#include "../ns/ns_proxy.h"
#include "../sec/audit.h"
int proc_unshare(u32 flags){ struct task* t=task_current(); if(!t||!t->owner) return -1; struct proc* p=t->owner; struct nsproxy* n=nsproxy_clone(p->nsproxy,flags,p->root_vnode); if(!n) return -1; nsproxy_put(p->nsproxy); p->nsproxy=n; p->pid_ns=pidns_alloc(n->pid); audit_log("ns.unshare"); return 0; }
