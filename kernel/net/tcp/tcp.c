#include "tcp.h"
#include "../../sched/spinlock.h"
#include "../../mem/mem.h"

struct tcp_conn g_tcp_conns[TCP_MAX_CONN];
spinlock_t g_tcp_lock;
struct tcp_stats g_tcp_stats;
u32 g_tcp_now;

void tcp_init(void){
    mem_set(g_tcp_conns,0,sizeof(g_tcp_conns));
    mem_set(&g_tcp_stats,0,sizeof(g_tcp_stats));
    spinlock_init(&g_tcp_lock);
    g_tcp_now=0;
}

const struct tcp_stats* tcp_stats_get(void){ return &g_tcp_stats; }
