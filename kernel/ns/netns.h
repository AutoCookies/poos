#ifndef POOS_NS_NETNS_H
#define POOS_NS_NETNS_H
#include "../types.h"
struct net_ns { u32 id; u32 refcnt; int allow_host_net; };
struct net_ns* netns_create(int allow_host_net);
void netns_get(struct net_ns* ns);
void netns_put(struct net_ns* ns);
#endif
