#include "userns.h"
#include "../mem/heap.h"
#include "ns.h"
struct user_ns* userns_create(u32 nsuid,u32 hostuid,u32 len){ struct user_ns* ns=(struct user_ns*)kmalloc(sizeof(*ns),8); if(!ns) return 0; ns->id=ns_next_id(); ns->refcnt=1; ns->ns_root_uid=nsuid; ns->host_uid=hostuid; ns->len=len?len:1; return ns; }
void userns_get(struct user_ns* ns){ if(ns) ns->refcnt++; }
void userns_put(struct user_ns* ns){ if(!ns) return; if(--ns->refcnt) return; kfree(ns); }
int userns_map_uid(struct user_ns* ns,u32 in,u32* out_host){ if(!ns||!out_host) return -1; if(in<ns->ns_root_uid||in>=ns->ns_root_uid+ns->len) return -1; *out_host=ns->host_uid+(in-ns->ns_root_uid); return 0; }
