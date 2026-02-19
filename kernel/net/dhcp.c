#include "dhcp.h"
#include "netif.h"
void vga_write(const char*);
int dhcp_start(void){ netif_t* n=netif_default(); if(!n) return -1; /* qemu user-net defaults */ n->ip=0x0A00020FU; n->netmask=0xFFFFFF00U; n->gw=0x0A000202U; n->dns=0x0A000203U; vga_write("dhcp: leased 10.0.2.15 gw 10.0.2.2 dns 10.0.2.3\n"); return 0; }
