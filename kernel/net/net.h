#ifndef POOS_NET_H
#define POOS_NET_H
#include "netif.h"
#include "pbuf.h"

void net_init(void);
void net_input(netif_t* n, pbuf_t* p);
int netif_register(netif_t* n);
void net_kick(void);
void net_print_stats(void);

#endif
