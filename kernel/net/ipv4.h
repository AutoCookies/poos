#ifndef POOS_IPV4_H
#define POOS_IPV4_H
#include "netif.h"
#include "pbuf.h"
void ipv4_input(netif_t* n, pbuf_t* p);
int ipv4_output(netif_t* n, u32 dst, u8 proto, pbuf_t* p);
#endif
