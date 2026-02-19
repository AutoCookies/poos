#ifndef POOS_ROUTE_H
#define POOS_ROUTE_H
#include "../types.h"
#include "netif.h"
u32 route_next_hop(netif_t* n, u32 dst);
#endif
