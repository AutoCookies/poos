#include "proc.h"
#include "task.h"
#include "../ns/ns.h"
int proc_unshare(u32 flags);
int proc_clone(u32 flags, struct trapframe* tf){ int pid=proc_fork_from_tf(tf); if(pid<0) return pid; if(flags){} return pid; }
int proc_setns(int fd,u32 nstype){ (void)fd; (void)nstype; return -1; }
