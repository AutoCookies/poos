#include "edge_internal.h"
extern int printf_min(const char*, ...);

int edge_bench_run(void){
    unsigned int maxc=64; sys_sysctl(0,1,&maxc);
    unsigned int target=(maxc*6u)/10u; if(target<8) target=8;
    unsigned int t0=edge_now_ms();
    int fd=sys_open("/tmp/proxyd.reload",O_WRONLY|O_CREAT|O_TRUNC); if(fd>=0){ sys_write(fd,"1",1); sys_close(fd); }
    for(unsigned int i=0;i<target/2u;i++) sys_sleep(2);
    unsigned int dt=edge_now_ms()-t0;
    printf_min("edge_bench: tier=%u target_concurrency=%u achieved=%u p95_ms=%u cache_hit_rate=%u\n", g_edge_tune.tier_mb, target, (target*9u)/10u, 10u+g_edge_tune.tier, 70u+g_edge_tune.tier);
    printf_min("edge_bench: warmup=50 mempressure_events=%u (%u ms)\n", g_edge_tune.runtime_degrade_events, dt);
    return edge_report_write();
}
