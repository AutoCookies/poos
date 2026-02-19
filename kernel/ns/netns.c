#include "netns.h"
#include "ns.h"
#include "ns_proxy.h"
#include "../mem/heap.h"
#include "../mem/mem.h"
#include "../proc/task.h"

static void netns_name(char out[8], const char* pfx, u32 id){
    out[0]=pfx[0]; out[1]=pfx[1]; out[2]=pfx[2];
    out[3]='0'+(char)((id/100)%10);
    out[4]='0'+(char)((id/10)%10);
    out[5]='0'+(char)(id%10);
    out[6]=0; out[7]=0;
}

static void netns_bootstrap_container(struct net_ns* ns){
    u32 oct = (ns->id % 240U) + 10U;
    netns_name(ns->veth_host, "vth", ns->id);
    netns_name(ns->veth_peer, "vtp", ns->id);
    netns_name(ns->bridge, "br0", ns->id);
    ns->bridge_ip = (10U<<24) | (200U<<16) | (oct<<8) | 1U;
    ns->bridge_mask = 0xFFFFFF00U;
    ns->container_ip = (10U<<24) | (200U<<16) | (oct<<8) | 2U;
    ns->dhcp_pool_next = (10U<<24) | (200U<<16) | (oct<<8) | 100U;
    ns->nat_enabled = 1;
    ns->allow_host_net = 1;
    ns->routes[0].dst = 0;
    ns->routes[0].mask = 0;
    ns->routes[0].gw = ns->bridge_ip;
    ns->routes[0].valid = 1;
}

struct net_ns* netns_create(int allow_host_net){
    struct net_ns* ns=(struct net_ns*)kmalloc(sizeof(*ns),8);
    if(!ns) return 0;
    mem_set(ns,0,sizeof(*ns));
    ns->id=ns_next_id();
    ns->refcnt=1;
    ns->allow_host_net=allow_host_net;
    if(!allow_host_net) netns_bootstrap_container(ns);
    return ns;
}

void netns_get(struct net_ns* ns){ if(ns) ns->refcnt++; }
void netns_put(struct net_ns* ns){ if(!ns) return; if(--ns->refcnt) return; kfree(ns); }

struct net_ns* netns_current(void){
    struct task* t = task_current();
    if(!t || !t->owner || !t->owner->nsproxy) return 0;
    return t->owner->nsproxy->net;
}

int netns_allowed(void){
    struct net_ns* ns = netns_current();
    return ns && ns->allow_host_net;
}

int netns_add_port_forward(struct net_ns* ns, u16 host_port, u32 container_ip, u16 container_port, u8 proto){
    if(!ns || !host_port || !container_port || !container_ip) return -1;
    for(int i=0;i<NETNS_PFWD_MAX;i++){
        if(ns->pfwd[i].valid && ns->pfwd[i].host_port==host_port && ns->pfwd[i].proto==proto) return -1;
    }
    for(int i=0;i<NETNS_PFWD_MAX;i++){
        if(!ns->pfwd[i].valid){
            ns->pfwd[i].valid=1;
            ns->pfwd[i].host_port=host_port;
            ns->pfwd[i].container_ip=container_ip;
            ns->pfwd[i].container_port=container_port;
            ns->pfwd[i].proto=proto;
            return 0;
        }
    }
    return -1;
}

struct netns_diag_u {
    u32 nsid;
    u32 bridge_ip;
    u32 bridge_mask;
    u32 container_ip;
    u32 nat_enabled;
    char veth_host[8];
    char veth_peer[8];
    char bridge[8];
    u32 bridge_rx;
    u32 bridge_tx;
    u32 nat_pkts;
    u32 nat_bytes;
    u32 pfwd_hits;
    u32 drops;
};

int netns_fill_diag(struct net_ns* ns, void* out, u32 len){
    if(!ns || !out || len < sizeof(struct netns_diag_u)) return -1;
    struct netns_diag_u* d = (struct netns_diag_u*)out;
    mem_set(d,0,sizeof(*d));
    d->nsid = ns->id;
    d->bridge_ip = ns->bridge_ip;
    d->bridge_mask = ns->bridge_mask;
    d->container_ip = ns->container_ip;
    d->nat_enabled = ns->nat_enabled;
    mem_copy(d->veth_host,ns->veth_host,8);
    mem_copy(d->veth_peer,ns->veth_peer,8);
    mem_copy(d->bridge,ns->bridge,8);
    d->bridge_rx = ns->stats.bridge_rx;
    d->bridge_tx = ns->stats.bridge_tx;
    d->nat_pkts = ns->stats.nat_pkts;
    d->nat_bytes = ns->stats.nat_bytes;
    d->pfwd_hits = ns->stats.pfwd_hits;
    d->drops = ns->stats.drops;
    return 0;
}
