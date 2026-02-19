#ifndef POOS_NETIF_H
#define POOS_NETIF_H
#include "../types.h"
struct pbuf;

typedef struct netif netif_t;
typedef int (*netif_tx_fn)(netif_t*, struct pbuf*);

struct netif {
    char name[8];
    u8 mac[6];
    u32 ip, netmask, gw, dns;
    u16 mtu;
    netif_tx_fn tx;
    void* driver;
    struct {
        u32 rx_packets, tx_packets, rx_drops, tx_drops;
        u32 malformed, arp_entries, icmp_echo_req, icmp_echo_reply;
        u32 udp_rx, udp_tx, udp_drop;
        u32 rxq_drop;
    } stats;
};

netif_t* netif_default(void);
void netif_set_default(netif_t* n);

#endif
