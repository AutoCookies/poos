#include "arp.h"
#include "eth.h"
#include "../mem/mem.h"
#include "../time/time.h"

#define ARP_MAX 32
struct arp_pkt{u16 htype,ptype;u8 hlen,plen;u16 oper;u8 sha[6];u32 spa;u8 tha[6];u32 tpa;} __attribute__((packed));
struct arp_ent{u32 ip;u8 mac[6];u32 ts;u8 valid;};
static struct arp_ent t[ARP_MAX]; static netif_t* nif;
static u16 sw16(u16 v){return (u16)((v>>8)|(v<<8));}
static u32 sw32(u32 v){return (v>>24)|((v>>8)&0xFF00)|((v<<8)&0xFF0000)|(v<<24);} 
void arp_init(void){ mem_set(t,0,sizeof(t)); }
void arp_set_local(netif_t* n){ nif=n; }
static void arp_learn(u32 ip,const u8 mac[6]){ u32 k=time_ticks(); int idx=-1; for(int i=0;i<ARP_MAX;i++){ if(t[i].valid&&t[i].ip==ip){idx=i;break;} if(idx<0 && !t[i].valid) idx=i; } if(idx<0) idx=0; t[idx].valid=1; t[idx].ip=ip; mem_copy(t[idx].mac,mac,6); t[idx].ts=k; if(nif) nif->stats.arp_entries++; }
static int arp_lookup(u32 ip,u8 mac[6]){ for(int i=0;i<ARP_MAX;i++) if(t[i].valid&&t[i].ip==ip){ mem_copy(mac,t[i].mac,6); return 0;} return -1; }
static int arp_send(netif_t* n,u16 op,const u8* tha,u32 tpa){ pbuf_t* p=pbuf_alloc(sizeof(struct arp_pkt),32); if(!p) return -1; struct arp_pkt* a=(struct arp_pkt*)p->data; a->htype=sw16(1); a->ptype=sw16(ETH_TYPE_IPV4); a->hlen=6; a->plen=4; a->oper=sw16(op); mem_copy(a->sha,n->mac,6); a->spa=sw32(n->ip); if(tha) mem_copy(a->tha,tha,6); else mem_set(a->tha,0,6); a->tpa=sw32(tpa); static const u8 bcast[6]={0xff,0xff,0xff,0xff,0xff,0xff}; return eth_output(n, op==1?bcast:tha, ETH_TYPE_ARP,p); }
int arp_resolve(netif_t* n,u32 ip,u8 mac[6],pbuf_t* hold,u16 et){ (void)et; if(arp_lookup(ip,mac)==0) return 0; arp_send(n,1,0,ip); if(hold){ pbuf_free(hold);} return -1; }
void arp_input(netif_t* n,pbuf_t* p){ if(p->len<sizeof(struct arp_pkt)){n->stats.malformed++; return;} struct arp_pkt* a=(struct arp_pkt*)p->data; if(sw16(a->htype)!=1||sw16(a->ptype)!=ETH_TYPE_IPV4) return; u32 spa=sw32(a->spa), tpa=sw32(a->tpa); arp_learn(spa,a->sha); u16 op=sw16(a->oper); if(op==1 && tpa==n->ip){ arp_send(n,2,a->sha,spa);} }
