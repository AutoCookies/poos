#ifndef POOS_NS_USERNS_H
#define POOS_NS_USERNS_H
#include "../types.h"
struct user_ns { u32 id; u32 refcnt; u32 ns_root_uid; u32 host_uid; u32 len; };
struct user_ns* userns_create(u32 nsuid,u32 hostuid,u32 len);
void userns_get(struct user_ns* ns);
void userns_put(struct user_ns* ns);
int userns_map_uid(struct user_ns* ns,u32 in,u32* out_host);
#endif
