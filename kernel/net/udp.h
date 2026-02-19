#ifndef POOS_UDP_H
#define POOS_UDP_H
#include "netif.h"
#include "pbuf.h"
void udp_input(netif_t* n, u32 sip, u32 dip, pbuf_t* p);
int udp_send(netif_t* n, u32 sip, u16 sport, u32 dip, u16 dport, const void* data, u16 len);
#endif
