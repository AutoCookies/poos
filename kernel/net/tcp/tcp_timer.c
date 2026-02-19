#include "tcp.h"

extern struct tcp_conn g_tcp_conns[TCP_MAX_CONN];
extern u32 g_tcp_now;
void tcp_retransmit_scan(void);

void tcp_set_now_ticks(u32 now){ g_tcp_now=now; }

void tcp_timer_tick(void){
    tcp_retransmit_scan();
    for(int i=0;i<TCP_MAX_CONN;i++){
        struct tcp_conn* c=&g_tcp_conns[i]; if(!c->used) continue;
        if(c->state==TCP_TIME_WAIT && (i32)(g_tcp_now-c->timewait_deadline)>=0) c->state=TCP_CLOSED;
    }
}
