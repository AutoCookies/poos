#include "tcp.h"
#include "../../sched/spinlock.h"
#include "../../mem/mem.h"

extern struct tcp_conn g_tcp_conns[TCP_MAX_CONN];
extern spinlock_t g_tcp_lock;
extern struct tcp_stats g_tcp_stats;

static u16 g_next_port = 49152;

static u32 hash4(u32 a,u16 b,u32 c,u16 d){ return (a^c^((u32)b<<16)^d) % TCP_MAX_CONN; }

struct tcp_conn* tcp_conn_lookup(u32 lip,u16 lport,u32 rip,u16 rport){
    spinlock_guard_t g = spin_lock_irqsave(&g_tcp_lock);
    u32 h = hash4(lip,lport,rip,rport);
    for(u32 i=0;i<TCP_MAX_CONN;i++){
        struct tcp_conn* c = &g_tcp_conns[(h+i)%TCP_MAX_CONN];
        if(c->used && c->local_ip==lip && c->local_port==lport && c->remote_ip==rip && c->remote_port==rport){
            spin_unlock_irqrestore(&g_tcp_lock,g);
            return c;
        }
    }
    spin_unlock_irqrestore(&g_tcp_lock,g);
    return 0;
}

struct tcp_conn* tcp_conn_alloc(void){
    spinlock_guard_t g = spin_lock_irqsave(&g_tcp_lock);
    for(u32 i=0;i<TCP_MAX_CONN;i++) if(!g_tcp_conns[i].used){
        struct tcp_conn* c=&g_tcp_conns[i];
        mem_set(c,0,sizeof(*c));
        c->used=1; c->state=TCP_CLOSED; c->recv_window=TCP_RX_BUF; c->send_window=TCP_MSS*4;
        g_tcp_stats.active++;
        spin_unlock_irqrestore(&g_tcp_lock,g);
        return c;
    }
    spin_unlock_irqrestore(&g_tcp_lock,g);
    return 0;
}

void tcp_conn_put(struct tcp_conn* c){
    if(!c) return;
    spinlock_guard_t g=spin_lock_irqsave(&g_tcp_lock);
    if(c->used){
        c->used=0;
        c->state=TCP_CLOSED;
        if(g_tcp_stats.active) g_tcp_stats.active--;
    }
    spin_unlock_irqrestore(&g_tcp_lock,g);
}

u16 tcp_alloc_ephemeral_port(void){
    spinlock_guard_t g=spin_lock_irqsave(&g_tcp_lock);
    u16 p=g_next_port++;
    if(g_next_port<49152) g_next_port=49152;
    spin_unlock_irqrestore(&g_tcp_lock,g);
    return p;
}
