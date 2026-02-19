#ifndef POOS_ARP_H
#define POOS_ARP_H
#include "netif.h"
#include "pbuf.h"
void arp_init(void);
void arp_input(netif_t* n, pbuf_t* p);
int arp_resolve(netif_t* n, u32 ip, u8 mac[6], pbuf_t* hold, u16 etype);
void arp_set_local(netif_t* n);
#endif
