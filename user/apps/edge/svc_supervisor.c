#include "edge_internal.h"
extern int printf_min(const char*, ...);

static struct edge_service_info g_svcs[EDGE_MAX_SERVICES];
static int g_nsvc = 0;
static unsigned int g_next_tune_poll_ms = 0;

static void sset(char* d,const char* s,int cap){ int i=0; for(;i<cap-1 && s[i];i++) d[i]=s[i]; d[i]=0; }
static int seq(const char* a,const char* b){ int i=0; for(;;i++){ if(a[i]!=b[i]) return 0; if(!a[i]) return 1; } }

void svc_supervisor_setup(void){
    g_nsvc = 1;
    sset(g_svcs[0].name, "proxyd", sizeof(g_svcs[0].name));
    sset(g_svcs[0].path, "/bin/proxyd", sizeof(g_svcs[0].path));
    g_svcs[0].pid = -1;
    g_svcs[0].state = EDGE_SVC_STATE_DOWN;
    g_svcs[0].backoff_ms = 50;
}

static int svc_spawn(struct edge_service_info* s){
    int pid = sys_spawn(s->path);
    if(pid>0){ s->pid=pid; s->state=EDGE_SVC_STATE_UP; return 0; }
    return -1;
}

static int proxyd_heartbeat_dead(unsigned int now_ms){
    int fd=sys_open("/tmp/proxyd.alive",O_RDONLY); if(fd<0) return 1;
    char b[24]; int n=sys_read(fd,b,23); sys_close(fd); if(n<=0) return 1; b[n]=0;
    unsigned int t=0; for(int i=0;b[i]>='0'&&b[i]<='9';i++) t=t*10u+(unsigned int)(b[i]-'0');
    return (now_ms>t) && (now_ms-t>1000u);
}

static void svc_maybe_restart(struct edge_service_info* s, unsigned int now_ms){
    if(s->pid>0 && !proxyd_heartbeat_dead(now_ms)) return;
    if(s->pid>0){
        sys_kill(s->pid,9);
        s->pid=-1; s->last_exit=9; s->last_crash_at=now_ms; s->restarts++;
        unsigned int minute = now_ms/60000u;
        if((unsigned int)s->restart_minute!=minute){ s->restart_minute=(int)minute; s->restarts=1; }
        if(s->restarts>10) s->state=EDGE_SVC_STATE_DEGRADED;
        s->next_restart_at = now_ms + s->backoff_ms;
        if(s->backoff_ms < 1000u) s->backoff_ms <<= 1;
    }
    if(now_ms < s->next_restart_at) return;
    if(s->state==EDGE_SVC_STATE_DEGRADED && s->restarts>10) return;
    svc_spawn(s);
}

void svc_supervisor_tick(void){
    unsigned int now_ms = edge_now_ms();
    if(now_ms >= g_next_tune_poll_ms){ edge_tune_tick(now_ms); g_next_tune_poll_ms = now_ms + edge_tune_poll_ms(); }
    for(int i=0;i<g_nsvc;i++) svc_maybe_restart(&g_svcs[i], now_ms);
}

int svc_supervisor_control(int argc, char** argv){
    if(argc<2) return -1;
    if(seq(argv[1],"list")){
        for(int i=0;i<g_nsvc;i++) printf_min("%s pid=%d state=%d restarts=%d\n", g_svcs[i].name, g_svcs[i].pid, g_svcs[i].state, g_svcs[i].restarts);
        return 0;
    }
    if(argc>=3 && seq(argv[1],"restart")){
        for(int i=0;i<g_nsvc;i++) if(seq(argv[2],g_svcs[i].name)){ if(g_svcs[i].pid>0) sys_kill(g_svcs[i].pid,9); g_svcs[i].pid=-1; g_svcs[i].next_restart_at=0; return 0; }
        return 1;
    }
    if(argc>=3 && seq(argv[1],"status")){
        for(int i=0;i<g_nsvc;i++) if(seq(argv[2],g_svcs[i].name)){ printf_min("%s pid=%d state=%d exit=%d crash_ms=%u\n",g_svcs[i].name,g_svcs[i].pid,g_svcs[i].state,g_svcs[i].last_exit,g_svcs[i].last_crash_at); return 0; }
        return 1;
    }
    return -1;
}

int svc_supervisor_snapshot(struct edge_service_info** out){ *out=g_svcs; return g_nsvc; }
