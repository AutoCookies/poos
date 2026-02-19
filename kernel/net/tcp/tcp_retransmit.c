#include "tcp.h"

extern struct tcp_conn g_tcp_conns[TCP_MAX_CONN];
extern struct tcp_stats g_tcp_stats;
extern u32 g_tcp_now;
int tcp_send_segment(struct tcp_conn* c,u32 seq,u32 ack,u8 flags,const u8* data,u16 len,int track);

void tcp_retransmit_scan(void){
    for(int i=0;i<TCP_MAX_CONN;i++){
        struct tcp_conn* c=&g_tcp_conns[i]; if(!c->used||!c->retransmit_q) continue;
        struct tcp_segq* q=c->retransmit_q;
        if((i32)(g_tcp_now - q->tx_deadline) >= 0){
            tcp_send_segment(c,q->seq,c->rcv_nxt,(u8)(q->flags|TCP_FLAG_ACK),q->data,q->len,0);
            g_tcp_stats.retransmits++;
            if(q->retries<10) q->retries++;
            q->tx_deadline = g_tcp_now + (100U << (q->retries>6?6:q->retries));
        }
    }
}
