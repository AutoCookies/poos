#include "ipv4.h"
#include "route.h"
#include "arp.h"
#include "eth.h"
#include "icmp.h"
#include "udp.h"
#include "checksum.h"
#include "../mem/mem.h"

struct ip4{u8 vihl,tos;u16 tot;u16 id;u16 frag;u8 ttl,proto;u16 csum;u32 sip,dip;} __attribute__((packed));
static u16 sw16(u16 v){return (u16)((v>>8)|(v<<8));}
static u32 sw32(u32 v){return (v>>24)|((v>>8)&0xFF00)|((v<<8)&0xFF0000)|(v<<24);} 

void ipv4_input(netif_t* n,pbuf_t* p){ if(p->len<20){n->stats.malformed++; return;} struct ip4* h=(struct ip4*)p->data; if((h->vihl>>4)!=4){n->stats.malformed++; return;} u32 ihl=(h->vihl&0xF)*4; if(ihl<20||p->len<ihl){n->stats.malformed++; return;} u16 tot=sw16(h->tot); if(tot<ihl||tot>p->len){n->stats.malformed++;return;} u16 old=h->csum; h->csum=0; if(net_checksum(h,ihl)!=old){ n->stats.rx_drops++; return; } h->csum=old; if((sw16(h->frag)&0x3FFF)!=0){ n->stats.rx_drops++; return; } u32 sip=sw32(h->sip),dip=sw32(h->dip); if(dip!=n->ip && dip!=0xFFFFFFFFU) return; p->len=tot; pbuf_pull(p,(u16)ihl); if(h->proto==1) icmp_input(n,sip,dip,p); else if(h->proto==17) udp_input(n,sip,dip,p); else n->stats.rx_drops++; }

int ipv4_output(netif_t* n,u32 dst,u8 proto,pbuf_t* p){ struct ip4* h=(struct ip4*)pbuf_push(p,20); if(!h) return -1; h->vihl=0x45; h->tos=0; h->tot=sw16(p->len); h->id=0; h->frag=0; h->ttl=64; h->proto=proto; h->sip=sw32(n->ip); h->dip=sw32(dst); h->csum=0; h->csum=net_checksum(h,20); u32 nh=route_next_hop(n,dst); u8 mac[6]; if(arp_resolve(n,nh,mac,p,ETH_TYPE_IPV4)<0){ n->stats.tx_drops++; return -1;} return eth_output(n,mac,ETH_TYPE_IPV4,p); }
