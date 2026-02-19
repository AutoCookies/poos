#ifndef POOS_NS_NETNS_H
#define POOS_NS_NETNS_H
#include "../types.h"

#define NETNS_ROUTE_MAX 8
#define NETNS_ARP_MAX 16
#define NETNS_PFWD_MAX 16

struct netns_route { u32 dst; u32 mask; u32 gw; u8 valid; };
struct netns_arp { u32 ip; u8 mac[6]; u8 valid; };
struct netns_pfwd {
    u16 host_port;
    u16 container_port;
    u32 container_ip;
    u8 proto;
    u8 valid;
    u32 packets;
    u32 bytes;
};

struct net_ns {
    u32 id;
    u32 refcnt;
    int allow_host_net;
    u8 nat_enabled;
    char veth_host[8];
    char veth_peer[8];
    char bridge[8];
    u32 bridge_ip;
    u32 bridge_mask;
    u32 container_ip;
    u32 dhcp_pool_next;
    struct netns_route routes[NETNS_ROUTE_MAX];
    struct netns_arp arp[NETNS_ARP_MAX];
    struct netns_pfwd pfwd[NETNS_PFWD_MAX];
    struct {
        u32 bridge_rx;
        u32 bridge_tx;
        u32 nat_pkts;
        u32 nat_bytes;
        u32 pfwd_hits;
        u32 drops;
    } stats;
};

struct net_ns* netns_create(int allow_host_net);
void netns_get(struct net_ns* ns);
void netns_put(struct net_ns* ns);
struct net_ns* netns_current(void);
int netns_allowed(void);
int netns_add_port_forward(struct net_ns* ns, u16 host_port, u32 container_ip, u16 container_port, u8 proto);
int netns_fill_diag(struct net_ns* ns, void* out, u32 len);
#endif
