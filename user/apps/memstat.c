#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);

int main(void){
    struct memstat_u st;
    if (sys_memstat(&st, sizeof(st)) < 0) {
        printf_min("memstat: unavailable\n");
        return 1;
    }
    static const char* names[8] = {
        "kernel_core","slab","page_cache","buffer_cache","network","tcp","tls","user_rss"
    };
    for (int i=0;i<8;i++) {
        printf_min("%s cap=%u used=%u peak=%u refused=%u\n", names[i], st.cat[i].cap, st.cat[i].used, st.cat[i].peak, st.cat[i].refused);
    }
    printf_min("drops=%u refused_conn=%u oom_kills=%u\n", st.dropped_packets, st.refused_connections, st.oom_kills);
    return 0;
}
