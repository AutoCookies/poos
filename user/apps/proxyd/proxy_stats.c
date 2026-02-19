#include "proxy_cache.h"
#include "proxy_stats.h"
extern int printf_min(const char*, ...);


static struct proxy_runtime_stats g_stats;

struct proxy_runtime_stats* proxy_stats_mut(void){ return &g_stats; }
void proxy_stats_dump(void){
    printf_min("conns=%u peak=%u req_ok=%u 4xx=%u 5xx=%u hit=%u miss=%u rl=%u timeout=%u shed=%u\n",
        g_stats.conns_current,g_stats.conns_peak,g_stats.req_ok,g_stats.req_4xx,g_stats.req_5xx,
        g_stats.cache_hit,g_stats.cache_miss,g_stats.rate_limited_count,g_stats.timeouts_count,g_stats.shed_load_count);
}
void proxy_stats_write_file(void){
    int fd=sys_open("/tmp/proxyd.stats.tmp",O_WRONLY|O_CREAT|O_TRUNC); if(fd<0) return;
    char b[256]; int p=0;
    unsigned int* v=(unsigned int*)&g_stats;
    for(int i=0;i<13;i++){
        if(i) b[p++]=' ';
        unsigned int x=v[i]; char t[12]; int tp=0; do{ t[tp++]=(char)('0'+(x%10)); x/=10; }while(x&&tp<11);
        for(int j=tp-1;j>=0;j--) b[p++]=t[j];
    }
    b[p++]='\n'; sys_write(fd,b,p); sys_close(fd); sys_rename("/tmp/proxyd.stats.tmp","/tmp/proxyd.stats");
}
