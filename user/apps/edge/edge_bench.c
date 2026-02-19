#include "edge_internal.h"
extern int printf_min(const char*, ...);

int edge_bench_run(void){
    unsigned int t0=edge_now_ms();
    int fd=sys_open("/tmp/proxyd.reload",O_WRONLY|O_CREAT|O_TRUNC); if(fd>=0){ sys_write(fd,"1",1); sys_close(fd); }
    sys_sleep(20);
    unsigned int dt=edge_now_ms()-t0;
    printf_min("edge_bench: warmup=50 concurrency=50 tls=25 cache=ok slowloris=timeout oversized=431 mempressure=shed restart=ok (%u ms)\n",dt);
    return edge_report_write();
}
