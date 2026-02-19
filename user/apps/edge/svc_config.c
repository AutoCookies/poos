#include "edge_internal.h"
extern int printf_min(const char*, ...);

static int slen(const char* s){ int n=0; while(s[n]) n++; return n; }

int edge_config_apply_defaults(void){
    int fd = sys_open("/etc/edge/proxyd.conf", O_RDONLY);
    if(fd>=0){ sys_close(fd); return 0; }
    sys_mkdir("/etc"); sys_mkdir("/etc/edge");
    fd = sys_open("/etc/edge/proxyd.conf", O_WRONLY|O_CREAT|O_TRUNC);
    if(fd<0) return -1;
    const char* d = "listen = 443\nupstream = 127.0.0.1:8080\ncache_mem = 8M\ncache_disk = 32M\nreq_hdr_max = 8192\nreq_line_max = 2048\ntimeout_header_ms = 3000\ntimeout_idle_ms = 15000\ntimeout_upstream_ms = 1500\nmax_conns = 64\nmax_tls_conns = 32\nrate_limit_rps = 20\nrate_limit_burst = 40\n";
    sys_write(fd,d,slen(d)); sys_close(fd);
    printf_min("edge: wrote default /etc/edge/proxyd.conf\n");
    return 0;
}

int edge_run_cmd(int argc, char** argv){
    if(argc<1) return -1;
    if(argv[0][0]=='s'){
        struct edge_service_info sv[EDGE_MAX_SERVICES]; int n=0; struct edge_health_state h;
        if(edge_status_read(sv,EDGE_MAX_SERVICES,&n,&h)<0) return 1;
        printf_min("health=%d proxyd_up=%d mem_low=%d\n",h.status,h.proxyd_up,h.mem_low);
        for(int i=0;i<n;i++) printf_min("svc %s pid=%d state=%d restarts=%d\n",sv[i].name,sv[i].pid,sv[i].state,sv[i].restarts);
        return 0;
    }
    return -1;
}
