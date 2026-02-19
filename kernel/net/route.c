#include "route.h"
u32 route_next_hop(netif_t* n,u32 dst){ return ((dst & n->netmask)==(n->ip & n->netmask))?dst:n->gw; }
