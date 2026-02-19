#include "proc.h"
#include "../sec/audit.h"
void exec_secure_apply(struct proc* p, struct vnode* file){ (void)file; if(!p||!p->cred) return; if(p->cred->euid!=0){ p->cred->cap_effective &= p->cred->cap_inheritable; } audit_log("exec"); }
