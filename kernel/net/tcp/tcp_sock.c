#include "tcp.h"
#include "../../sched/sched.h"

u32 tcp_window_sendable(struct tcp_conn* c);
int tcp_send_segment(struct tcp_conn* c,u32 seq,u32 ack,u8 flags,const u8* data,u16 len,int track);

int tcp_connect(struct tcp_conn* c, u32 rip, u16 rport){
    netif_t* n=netif_default(); if(!c||!n) return -1;
    c->local_ip=n->ip; c->remote_ip=rip; c->remote_port=rport;
    c->iss=((u32)c->local_port<<16)^rip^rport;
    c->snd_una=c->iss; c->snd_nxt=c->iss+1; c->state=TCP_SYN_SENT;
    if(tcp_send_segment(c,c->iss,0,TCP_FLAG_SYN,0,0,1)<0) return -1;
    for(int t=0;t<5000;t++){
        if(c->state==TCP_ESTABLISHED) return 0;
        if(c->state==TCP_CLOSED) return -1;
        kthread_sleep(1);
    }
    return -1;
}

int tcp_send(struct tcp_conn* c, const u8* buf, u32 len){
    if(!c||c->state!=TCP_ESTABLISHED) return -1;
    u32 off=0;
    while(off<len){
        u32 can=tcp_window_sendable(c); if(!can){ kthread_sleep(1); continue; }
        u16 n=(u16)((len-off)>TCP_MSS?TCP_MSS:(len-off)); if(n>can) n=(u16)can;
        if(tcp_send_segment(c,c->snd_nxt,c->rcv_nxt,TCP_FLAG_ACK|TCP_FLAG_PSH,buf+off,n,1)<0) return -1;
        c->snd_nxt += n; off += n;
    }
    return (int)len;
}

int tcp_recv(struct tcp_conn* c, u8* buf, u32 len){
    if(!c||!buf) return -1;
    for(;;){
        if(c->rx_count){
            u32 n=(len<c->rx_count)?len:c->rx_count;
            for(u32 i=0;i<n;i++){ buf[i]=c->rxbuf[c->rx_head]; c->rx_head=(u16)((c->rx_head+1)%TCP_RX_BUF); }
            c->rx_count=(u16)(c->rx_count-n);
            return (int)n;
        }
        if(c->state==TCP_CLOSE_WAIT || c->state==TCP_CLOSED || c->state==TCP_TIME_WAIT) return 0;
        kthread_sleep(1);
    }
}

int tcp_close(struct tcp_conn* c){
    if(!c) return -1;
    if(c->state==TCP_ESTABLISHED){
        c->state=TCP_FIN_WAIT_1;
        tcp_send_segment(c,c->snd_nxt,c->rcv_nxt,TCP_FLAG_FIN|TCP_FLAG_ACK,0,0,1);
        c->snd_nxt++;
    } else if(c->state==TCP_CLOSE_WAIT){
        c->state=TCP_LAST_ACK;
        tcp_send_segment(c,c->snd_nxt,c->rcv_nxt,TCP_FLAG_FIN|TCP_FLAG_ACK,0,0,1);
        c->snd_nxt++;
    }
    for(int t=0;t<3000;t++){
        if(c->state==TCP_CLOSED) break;
        if(c->state==TCP_TIME_WAIT){ c->timewait_deadline+=200; }
        kthread_sleep(1);
    }
    tcp_conn_put(c);
    return 0;
}
