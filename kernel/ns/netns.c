#include "netns.h"
#include "../mem/heap.h"
#include "ns.h"
struct net_ns* netns_create(int allow_host_net){ struct net_ns* ns=(struct net_ns*)kmalloc(sizeof(*ns),8); if(!ns) return 0; ns->id=ns_next_id(); ns->refcnt=1; ns->allow_host_net=allow_host_net; return ns; }
void netns_get(struct net_ns* ns){ if(ns) ns->refcnt++; }
void netns_put(struct net_ns* ns){ if(!ns) return; if(--ns->refcnt) return; kfree(ns); }
