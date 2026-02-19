#include "utsns.h"
#include "../mem/heap.h"
#include "../mem/mem.h"
#include "ns.h"
static void cpy(char* d,const char* s){ u32 i=0; if(!s){ d[0]=0; return; } for(;i<63&&s[i];++i)d[i]=s[i]; d[i]=0; }
struct uts_ns* utsns_create(const char* host){ struct uts_ns* ns=(struct uts_ns*)kmalloc(sizeof(*ns),8); if(!ns) return 0; ns->id=ns_next_id(); ns->refcnt=1; cpy(ns->hostname, host?host:"poos"); return ns; }
void utsns_get(struct uts_ns* ns){ if(ns) ns->refcnt++; }
void utsns_put(struct uts_ns* ns){ if(!ns) return; if(--ns->refcnt) return; kfree(ns); }
int utsns_sethostname(struct uts_ns* ns,const char* host){ if(!ns||!host) return -1; cpy(ns->hostname,host); return 0; }
