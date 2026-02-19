#include "pidns.h"
#include "../mem/heap.h"
#include "ns.h"
struct pid_ns* pidns_create(void){ struct pid_ns* ns=(struct pid_ns*)kmalloc(sizeof(*ns),8); if(!ns) return 0; ns->id=ns_next_id(); ns->refcnt=1; ns->next_pid=1; return ns; }
void pidns_get(struct pid_ns* ns){ if(ns) ns->refcnt++; }
void pidns_put(struct pid_ns* ns){ if(!ns) return; if(--ns->refcnt) return; kfree(ns); }
u32 pidns_alloc(struct pid_ns* ns){ if(!ns) return 0; return ns->next_pid++; }
