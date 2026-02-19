#include "eth.h"
#include "arp.h"
#include "ipv4.h"
#include "../mem/mem.h"

struct eth_hdr{u8 dst[6],src[6];u16 type;} __attribute__((packed));
static u16 bswap16(u16 v){return (u16)((v>>8)|(v<<8));}

int eth_output(netif_t* n,const u8* dst,u16 et,pbuf_t* p){ struct eth_hdr* h=(struct eth_hdr*)pbuf_push(p,sizeof(*h)); if(!h) return -1; mem_copy(h->dst,dst,6); mem_copy(h->src,n->mac,6); h->type=bswap16(et); n->stats.tx_packets++; return n->tx(n,p); }
void eth_input(netif_t* n,pbuf_t* p){ if(p->len<sizeof(struct eth_hdr)){n->stats.malformed++;return;} struct eth_hdr* h=(struct eth_hdr*)p->data; u16 t=bswap16(h->type); pbuf_pull(p,sizeof(*h)); n->stats.rx_packets++; if(t==ETH_TYPE_ARP) arp_input(n,p); else if(t==ETH_TYPE_IPV4) ipv4_input(n,p); else n->stats.rx_drops++; }
