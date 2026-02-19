#include "tcp.h"
#include "../../mem/mem.h"
#include "../../mem/heap.h"

extern struct tcp_stats g_tcp_stats;
int tcp_send_segment(struct tcp_conn* c,u32 seq,u32 ack,u8 flags,const u8* data,u16 len,int track);
int tcp_send_ack(struct tcp_conn* c);

struct tcph{u16 sport,dport;u32 seq,ack;u8 off;u8 flags;u16 win;u16 sum;u16 urg;} __attribute__((packed));
static u16 sw16(u16 v){return (u16)((v>>8)|(v<<8));}
static u32 sw32(u32 v){return (v>>24)|((v>>8)&0xFF00)|((v<<8)&0xFF0000)|(v<<24);} 
static u16 tcp_checksum(u32 sip,u32 dip,const u8* seg,u16 len){ u32 sum=0; sum += (sip>>16)&0xFFFF; sum += sip&0xFFFF; sum += (dip>>16)&0xFFFF; sum += dip&0xFFFF; sum += 6; sum += len; for(u16 i=0;i+1<len;i+=2) sum += ((u16)seg[i]<<8)|seg[i+1]; if(len&1) sum += ((u16)seg[len-1]<<8); while(sum>>16) sum=(sum&0xFFFF)+(sum>>16); return (u16)~sum; }

static void tcp_ack_advance(struct tcp_conn* c, u32 ack){
    if(ack<=c->snd_una || ack>c->snd_nxt) return;
    c->snd_una=ack;
    while(c->retransmit_q){
        struct tcp_segq* q=c->retransmit_q;
        u32 end=q->seq+q->len+((q->flags&(TCP_FLAG_SYN|TCP_FLAG_FIN))?1:0);
        if(end<=ack){ c->retransmit_q=q->next; kfree(q); }
        else break;
    }
}

void tcp_input(netif_t* n,u32 sip,u32 dip,pbuf_t* p){
    (void)n;
    if(p->len<20){ g_tcp_stats.drops++; return; }
    struct tcph* h=(struct tcph*)p->data; u16 hlen=(u16)((h->off>>4)*4);
    if(hlen<20 || hlen>p->len){ g_tcp_stats.drops++; return; }
    u16 old=h->sum; h->sum=0; if(tcp_checksum(sip,dip,p->data,p->len)!=old){ g_tcp_stats.drops++; return; } h->sum=old;
    u16 dport=sw16(h->dport),sport=sw16(h->sport); u32 seq=sw32(h->seq),ack=sw32(h->ack);
    struct tcp_conn* c=tcp_conn_lookup(dip,dport,sip,sport); if(!c){ g_tcp_stats.rst++; return; }
    g_tcp_stats.seg_rx++; c->send_window=sw16(h->win);
    u16 dlen=(u16)(p->len-hlen); u8* d=p->data+hlen;
    if(h->flags & TCP_FLAG_RST){ c->state=TCP_CLOSED; return; }
    if(c->state==TCP_SYN_SENT){
        if((h->flags&(TCP_FLAG_SYN|TCP_FLAG_ACK))==(TCP_FLAG_SYN|TCP_FLAG_ACK) && ack==c->snd_nxt){
            c->irs=seq; c->rcv_nxt=seq+1; c->snd_una=ack; c->state=TCP_ESTABLISHED; tcp_send_ack(c); return;
        }
        return;
    }
    if((h->flags&TCP_FLAG_ACK)) tcp_ack_advance(c,ack);
    if(dlen){
        if(seq==c->rcv_nxt){
            u16 space=(u16)(TCP_RX_BUF-c->rx_count); if(dlen>space) dlen=space;
            for(u16 i=0;i<dlen;i++){ c->rxbuf[c->rx_tail]=d[i]; c->rx_tail=(u16)((c->rx_tail+1)%TCP_RX_BUF); }
            c->rx_count=(u16)(c->rx_count+dlen); c->rcv_nxt += dlen;
            tcp_send_ack(c);
        } else {
            g_tcp_stats.ooo++;
        }
    }
    if(h->flags & TCP_FLAG_FIN){
        if(seq + dlen == c->rcv_nxt){ c->rcv_nxt++; tcp_send_ack(c); }
        if(c->state==TCP_ESTABLISHED) c->state=TCP_CLOSE_WAIT;
        else if(c->state==TCP_FIN_WAIT_1) c->state=TCP_TIME_WAIT;
        else if(c->state==TCP_FIN_WAIT_2) c->state=TCP_TIME_WAIT;
    }
    if(c->state==TCP_LAST_ACK && c->snd_una==c->snd_nxt) c->state=TCP_CLOSED;
    if(c->state==TCP_FIN_WAIT_1 && c->snd_una==c->snd_nxt) c->state=TCP_FIN_WAIT_2;
}
