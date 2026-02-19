#include "edge_internal.h"
extern int printf_min(const char*, ...);

int edge_healthcheck(int verbose){
    struct edge_service_info sv[EDGE_MAX_SERVICES]; int n=0; struct edge_health_state h;
    if(edge_status_read(sv,EDGE_MAX_SERVICES,&n,&h)<0){ if(verbose) printf_min("FAIL\n"); return 1; }
    const char* s = (h.status==0)?"OK":((h.status==1)?"DEGRADED":"FAIL");
    if(verbose) printf_min("%s\n",s);
    return h.status==2;
}
