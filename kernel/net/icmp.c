#include "icmp.h"
#include "ipv4.h"
#include "checksum.h"
#include "../mem/mem.h"
#include "../time/time.h"
#include "../sched/sched.h"
struct icmph{u8 type,code;u16 csum,id,seq;} __attribute__((packed));
static u16 sw16(u16 v){return (u16)((v>>8)|(v<<8));}
static volatile u16 wait_id, wait_seq; static volatile int wait_done;
void icmp_input(netif_t* n,u32 sip,u32 dip,pbuf_t* p){ (void)dip; if(p->len<8){n->stats.malformed++;return;} struct icmph* h=(struct icmph*)p->data; if(net_checksum(h,p->len)!=0) return; if(h->type==8){ h->type=0; h->csum=0; h->csum=net_checksum(h,p->len); n->stats.icmp_echo_req++; ipv4_output(n,sip,1,p); }
 else if(h->type==0){ n->stats.icmp_echo_reply++; if(sw16(h->id)==wait_id && sw16(h->seq)==wait_seq){ wait_done=1; } }
}
int icmp_ping(u32 dst,u16 id,u16 seq,u32 timeout_ms){ netif_t* n=netif_default(); if(!n) return -1; pbuf_t* p=pbuf_alloc(8,32); if(!p) return -1; struct icmph* h=(struct icmph*)p->data; h->type=8; h->code=0; h->id=sw16(id); h->seq=sw16(seq); h->csum=0; h->csum=net_checksum(h,8); wait_id=id; wait_seq=seq; wait_done=0; if(ipv4_output(n,dst,1,p)<0){pbuf_free(p); return -1;} u32 until=time_ticks()+time_ms_to_ticks(timeout_ms); while(!wait_done && time_ticks()<until) kthread_sleep(1); return wait_done?0:-1; }
