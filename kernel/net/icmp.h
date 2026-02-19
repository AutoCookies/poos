#ifndef POOS_ICMP_H
#define POOS_ICMP_H
#include "netif.h"
#include "pbuf.h"
void icmp_input(netif_t* n, u32 sip, u32 dip, pbuf_t* p);
int icmp_ping(u32 dst, u16 id, u16 seq, u32 timeout_ms);
#endif
