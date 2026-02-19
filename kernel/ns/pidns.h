#ifndef POOS_NS_PIDNS_H
#define POOS_NS_PIDNS_H
#include "../types.h"
struct pid_ns { u32 id; u32 refcnt; u32 next_pid; };
struct pid_ns* pidns_create(void);
void pidns_get(struct pid_ns* ns);
void pidns_put(struct pid_ns* ns);
u32 pidns_alloc(struct pid_ns* ns);
#endif
