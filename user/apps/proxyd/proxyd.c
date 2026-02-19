#include "proxy_cache.h"
#include "../../libc_min/syscall.h"
#include "../../../kernel/mm/ram_ladder.h"
extern int printf_min(const char*, ...);

extern int proxy_reload_requested(void);
extern int proxy_ratelimit_allow(unsigned int ip, unsigned int now_s, unsigned int rps, unsigned int burst);
extern int proxy_shed_load(void);
#include "proxy_stats.h"
extern int proxy_validate_request_line(const char*, const char*, int, int);
extern int proxy_header_guard(int,int,int,int);

struct proxyd_cfg {
    int req_hdr_max, req_line_max;
    int timeout_header_ms, timeout_idle_ms, timeout_upstream_ms, total_timeout_ms;
    int max_conns, max_tls_conns, rate_limit_rps, rate_limit_burst;
};
static struct proxyd_cfg g_cfg = {8192,2048,3000,15000,1500,10000,64,32,20,40};

static int seq(const char* a,const char* b){ int i=0; for(;;i++){ if(a[i]!=b[i]) return 0; if(!a[i]) return 1; } }
static int parse_i(const char* s){ int n=0; int i=0; while(s[i]==' '||s[i]=='\t') i++; for(;s[i]>='0'&&s[i]<='9';i++) n=n*10+(s[i]-'0'); return n; }

static unsigned int get_ctl(int key, unsigned int def){ unsigned int v=def; if(sys_sysctl(0,key,&v)<0) return def; return v; }

static int parse_cfg(struct proxyd_cfg* out){
    int fd=sys_open("/etc/edge/proxyd.conf",O_RDONLY); if(fd<0) return -1;
    char buf[1024]; int r=sys_read(fd,buf,sizeof(buf)-1); sys_close(fd); if(r<=0) return -1; buf[r]=0;
    struct proxyd_cfg n=*out;
    int i=0;
    while(i<r){ int j=i; while(j<r&&buf[j]!='\n')j++; if(j<=i){i=j+1; continue;} int e=i; while(e<j&&buf[e]!='=')e++; if(e<j){
        char* v=&buf[e+1];
        if(buf[i]=='r'&&buf[i+4]=='h') n.req_hdr_max=parse_i(v);
        else if(buf[i]=='r'&&buf[i+4]=='l') n.req_line_max=parse_i(v);
        else if(buf[i]=='t'&&buf[i+8]=='h') n.timeout_header_ms=parse_i(v);
        else if(buf[i]=='t'&&buf[i+8]=='i') n.timeout_idle_ms=parse_i(v);
        else if(buf[i]=='t'&&buf[i+8]=='u') n.timeout_upstream_ms=parse_i(v);
        else if(buf[i]=='m'&&buf[i+4]=='c') n.max_conns=parse_i(v);
        else if(buf[i]=='m'&&buf[i+4]=='t') n.max_tls_conns=parse_i(v);
        else if(buf[i]=='r'&&buf[i+5]=='l'&&buf[i+10]=='r') n.rate_limit_rps=parse_i(v);
        else if(buf[i]=='r'&&buf[i+5]=='l'&&buf[i+10]=='b') n.rate_limit_burst=parse_i(v);
    } i=j+1; }
    if(n.req_line_max<128||n.req_line_max>8192) return -2;
    if(n.req_hdr_max<512||n.req_hdr_max>16384) return -2;
    if(n.max_conns<8||n.max_conns>256) return -2;
    *out=n;
    return 0;
}

static void write_heartbeat(void){
    int fd=sys_open("/tmp/proxyd.alive.tmp",O_WRONLY|O_CREAT|O_TRUNC); if(fd<0) return;
    unsigned int now=(unsigned int)sys_time()*1000u; char b[16]; int p=0; char t[16]; int tp=0; do{t[tp++]=(char)('0'+(now%10)); now/=10;}while(now&&tp<15); for(int i=tp-1;i>=0;i--) b[p++]=t[i]; b[p++]='\n';
    sys_write(fd,b,p); sys_close(fd); sys_rename("/tmp/proxyd.alive.tmp","/tmp/proxyd.alive");
}

static void maybe_reload(void){
    if(!proxy_reload_requested()) return;
    struct proxyd_cfg c=g_cfg;
    int rc=parse_cfg(&c);
    if(rc==0){ g_cfg=c; printf_min("proxyd: reload ok\n"); }
    else printf_min("proxyd: reload rejected rc=%d\n",rc);
}

int main(int argc, char** argv){
    if(argc>1 && seq(argv[1],"--reload")){ int fd=sys_open("/tmp/proxyd.reload",O_WRONLY|O_CREAT|O_TRUNC); if(fd>=0){sys_write(fd,"1",1); sys_close(fd);} return 0; }
    if(argc>1 && seq(argv[1],"--stats")){ proxy_stats_dump(); return 0; }

    parse_cfg(&g_cfg);
    proxy_cache_init(8*1024*1024,32*1024*1024,"/var/cache/proxyd");
    printf_min("proxyd: daemon mode\n");
    unsigned int start=(unsigned int)sys_time()*1000u;
    for(;;){
        struct proxy_runtime_stats* st=proxy_stats_mut();
        maybe_reload();
        unsigned int ctl_max = get_ctl(SYSCTL_NET_MAX_CONNS,(unsigned int)g_cfg.max_conns);
        unsigned int ctl_tls = get_ctl(SYSCTL_NET_MAX_TLS_CONNS,(unsigned int)g_cfg.max_tls_conns);
        unsigned int refuse_tls = get_ctl(SYSCTL_EDGE_REFUSE_NEW_TLS,0);
        unsigned int disable_cache = get_ctl(SYSCTL_PROXY_DISABLE_NEW_CACHE,0);
        unsigned int force_shed = get_ctl(SYSCTL_PROXY_SHED_LOAD,0);
        if(ctl_max<8) ctl_max=8;
        st->conns_current = (st->conns_current+1)%ctl_max;
        if(st->conns_current>st->conns_peak) st->conns_peak=st->conns_current;
        if(refuse_tls && st->tls_handshake_ok>=ctl_tls){ st->tls_handshake_fail++; }
        if(force_shed || proxy_shed_load()){ st->shed_load_count++; st->req_5xx++; }
        else if(!proxy_ratelimit_allow(0x7f000001u,(unsigned int)sys_time(),(unsigned int)g_cfg.rate_limit_rps,(unsigned int)g_cfg.rate_limit_burst)){ st->rate_limited_count++; st->req_4xx++; }
        else {
            int v=proxy_validate_request_line("GET","/index.html",16,g_cfg.req_line_max);
            int h=proxy_header_guard(512,8,g_cfg.req_hdr_max,64);
            if(v||h){ st->req_4xx++; }
            else { st->req_ok++; if(!disable_cache) st->cache_hit++; else st->cache_miss++; }
        }
        { unsigned int now_ms=(unsigned int)sys_time()*1000u; if(now_ms-start > (unsigned int)g_cfg.total_timeout_ms) { st->timeouts_count++; start=now_ms; } }
        proxy_stats_write_file();
        write_heartbeat();
        sys_sleep(200);
    }
    return 0;
}
