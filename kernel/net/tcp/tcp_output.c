#include "tcp.h"
#include "../ipv4.h"
#include "../checksum.h"
#include "../../mem/mem.h"
#include "../../mem/heap.h"

extern struct tcp_stats g_tcp_stats;
extern u32 g_tcp_now;

struct tcph{u16 sport,dport;u32 seq,ack;u8 off;u8 flags;u16 win;u16 sum;u16 urg;} __attribute__((packed));
static u16 sw16(u16 v){return (u16)((v>>8)|(v<<8));}
static u32 sw32(u32 v){return (v>>24)|((v>>8)&0xFF00)|((v<<8)&0xFF0000)|(v<<24);} 

static u16 tcp_checksum(u32 sip,u32 dip,const u8* seg,u16 len){
    u32 sum=0; sum += (sip>>16)&0xFFFF; sum += sip&0xFFFF; sum += (dip>>16)&0xFFFF; sum += dip&0xFFFF; sum += 6; sum += len;
    for(u16 i=0;i+1<len;i+=2) sum += ((u16)seg[i]<<8)|seg[i+1];
    if(len&1) sum += ((u16)seg[len-1]<<8);
    while(sum>>16) sum=(sum&0xFFFF)+(sum>>16);
    return (u16)~sum;
}

int tcp_send_segment(struct tcp_conn* c, u32 seq, u32 ack, u8 flags, const u8* data, u16 len, int track){
    netif_t* n=netif_default(); if(!n||!c) return -1;
    pbuf_t* p=pbuf_alloc((u16)(20+len),64); if(!p) return -1;
    struct tcph* h=(struct tcph*)pbuf_push(p,20); if(!h){pbuf_free(p);return -1;}
    h->sport=sw16(c->local_port); h->dport=sw16(c->remote_port);
    h->seq=sw32(seq); h->ack=sw32(ack); h->off=(5<<4); h->flags=flags;
    h->win=sw16((u16)(TCP_RX_BUF-c->rx_count)); h->urg=0; h->sum=0;
    if(len && data) mem_copy(p->data+20,data,len);
    h->sum=tcp_checksum(c->local_ip,c->remote_ip,p->data,p->len);
    if(ipv4_output(n,c->remote_ip,6,p)<0){ pbuf_free(p); return -1; }
    g_tcp_stats.seg_tx++;
    if(track && (len || (flags&(TCP_FLAG_SYN|TCP_FLAG_FIN)))){
        struct tcp_segq* q=(struct tcp_segq*)kmalloc(sizeof(*q),8); if(!q) return 0;
        q->seq=seq; q->len=len; q->flags=flags; q->tx_deadline=g_tcp_now+100; q->retries=0; q->next=0;
        if(len&&data) mem_copy(q->data,data,len);
        if(!c->retransmit_q) c->retransmit_q=q; else { struct tcp_segq* t=c->retransmit_q; while(t->next)t=t->next; t->next=q; }
    }
    return 0;
}

int tcp_send_ack(struct tcp_conn* c){ return tcp_send_segment(c,c->snd_nxt,c->rcv_nxt,TCP_FLAG_ACK,0,0,0); }
