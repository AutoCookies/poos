#include "udp.h"
#include "sock.h"
#include "ipv4.h"
#include "checksum.h"
#include "../mem/mem.h"
struct udph{u16 sport,dport,len,csum;} __attribute__((packed));
static u16 sw16(u16 v){return (u16)((v>>8)|(v<<8));}
void udp_input(netif_t* n,u32 sip,u32 dip,pbuf_t* p){ if(p->len<8){n->stats.malformed++;return;} struct udph* h=(struct udph*)p->data; u16 ulen=sw16(h->len); if(ulen<8||ulen>p->len){n->stats.malformed++;return;} p->len=ulen; u16 sport=sw16(h->sport), dport=sw16(h->dport); pbuf_pull(p,8); sock_udp_deliver(sip,sport,dip,dport,p); n->stats.udp_rx++; }
int udp_send(netif_t* n,u32 sip,u16 sport,u32 dip,u16 dport,const void* data,u16 len){ (void)sip; pbuf_t* p=pbuf_alloc(len,64); if(!p) return -1; mem_copy(p->data,data,len); struct udph* h=(struct udph*)pbuf_push(p,8); if(!h){pbuf_free(p);return -1;} h->sport=sw16(sport); h->dport=sw16(dport); h->len=sw16(p->len); h->csum=0; h->csum=net_checksum_udp(n->ip,dip,17,h,p->len); if(h->csum==0) h->csum=0xFFFF; n->stats.udp_tx++; return ipv4_output(n,dip,17,p); }
