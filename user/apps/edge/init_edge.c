#include "edge_internal.h"
extern int printf_min(const char*, ...);

int edge_config_apply_defaults(void);
int edge_run_cmd(int argc, char** argv);

static int seq(const char* a,const char* b){ int i=0; for(;;i++){ if(a[i]!=b[i]) return 0; if(!a[i]) return 1; } }

int main(int argc, char** argv){
    if(argc>1 && seq(argv[1],"healthcheck")) return edge_healthcheck(1);
    if(argc>1 && seq(argv[1],"run-golden")){
        printf_min("edge run-golden profile=edge80-prox ram=80MB\n");
        edge_healthcheck(1);
        if(edge_bench_run()==0) printf_min("edge: report /var/log/edge-report.json (fallback /tmp/edge-report.json)\n");
        return 0;
    }
    if(argc>1 && seq(argv[1],"svc")) return edge_run_cmd(argc-2, argv+2);

    edge_config_apply_defaults();
    svc_supervisor_setup();
    printf_min("edge: supervisor online\n");
    for(;;){
        svc_supervisor_tick();
        struct edge_service_info* sv=0; int n=svc_supervisor_snapshot(&sv);
        struct edge_health_state h; h.status=0; h.proxyd_up=0; h.restart_storm=0; h.mem_low=edge_mem_free_low(); h.low_mem_ticks = h.mem_low?1:0;
        for(int i=0;i<n;i++) if(sv[i].name[0]=='p') { if(sv[i].pid>0) h.proxyd_up=1; if(sv[i].state==EDGE_SVC_STATE_DEGRADED) h.restart_storm=1; }
        if(!h.proxyd_up) h.status=2; else if(h.mem_low||h.restart_storm) h.status=1;
        edge_status_write(sv,n,&h);
        sys_sleep(100);
    }
    return 0;
}
