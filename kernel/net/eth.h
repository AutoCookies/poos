#ifndef POOS_ETH_H
#define POOS_ETH_H
#include "netif.h"
#include "pbuf.h"
#define ETH_TYPE_ARP 0x0806
#define ETH_TYPE_IPV4 0x0800
int eth_output(netif_t* n, const u8* dst, u16 et, pbuf_t* p);
void eth_input(netif_t* n, pbuf_t* p);
#endif
