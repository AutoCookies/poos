#include "../libc_min/syscall.h"
#include "../../kernel/mm/ram_ladder.h"
extern int printf_min(const char*, ...);

int main(void){
    struct memstat_u st;
    struct meminfo_u mi;
    if (sys_memstat(&st, sizeof(st)) < 0) {
        printf_min("memstat: unavailable\n");
        return 1;
    }
    if (sys_meminfo(&mi,sizeof(mi))<0){ mi.total_bytes=0; mi.free_bytes=0; }
    unsigned int tier=0, maxc=0, ka=0; sys_sysctl(0,SYSCTL_EDGE_TIER_MB,&tier); sys_sysctl(0,SYSCTL_NET_MAX_CONNS,&maxc); sys_sysctl(0,SYSCTL_PROXY_KEEPALIVE_MS,&ka);
    printf_min("ram_total_mb=%u ram_free_mb=%u chosen_tier_log=%u max_conns=%u keepalive_ms=%u\n", mi.total_bytes/(1024u*1024u), mi.free_bytes/(1024u*1024u), tier, maxc, ka);
    static const char* names[8] = {"kernel_core","slab","page_cache","buffer_cache","network","tcp","tls","user_rss"};
    for (int i=0;i<8;i++) printf_min("%s cap=%u used=%u peak=%u refused=%u\n", names[i], st.cat[i].cap, st.cat[i].used, st.cat[i].peak, st.cat[i].refused);
    printf_min("drops=%u refused_conn=%u oom_kills=%u\n", st.dropped_packets, st.refused_connections, st.oom_kills);
    return 0;
}
