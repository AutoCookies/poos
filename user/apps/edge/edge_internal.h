#ifndef EDGE_INTERNAL_H
#define EDGE_INTERNAL_H

#include "../../libc_min/syscall.h"

#define EDGE_MAX_SERVICES 8
#define EDGE_SVC_STATE_DOWN 0
#define EDGE_SVC_STATE_UP 1
#define EDGE_SVC_STATE_DEGRADED 2

struct edge_health_state {
    int status; /* 0 ok, 1 degraded, 2 fail */
    int proxyd_up;
    int restart_storm;
    int mem_low;
    unsigned int low_mem_ticks;
};


struct edge_tune_state {
    unsigned int ram_total_mb;
    unsigned int tier_mb;
    unsigned int max_conns_cur;
    unsigned int min_free_mb;
    unsigned int runtime_degrade_events;
    unsigned int shed_load_count;
    unsigned int cooldown_until_ms;
    int tier;
    int pressure_level;
};
extern struct edge_tune_state g_edge_tune;

struct edge_service_info {
    char name[16];
    char path[32];
    int pid;
    int state;
    int restarts;
    int last_exit;
    int restart_minute;
    unsigned int backoff_ms;
    unsigned int next_restart_at;
    unsigned int last_crash_at;
};

int edge_status_write(const struct edge_service_info* svcs, int n, const struct edge_health_state* h);
int edge_status_read(struct edge_service_info* svcs, int cap, int* n, struct edge_health_state* h);
unsigned int edge_now_ms(void);
int edge_mem_free_low(void);

void svc_supervisor_setup(void);
void svc_supervisor_tick(void);
int svc_supervisor_control(int argc, char** argv);
int svc_supervisor_snapshot(struct edge_service_info** out);
int edge_bench_run(void);
int edge_report_write(void);
int edge_healthcheck(int verbose);
int edge_tune_bootstrap(int forced_tier_mb);
void edge_tune_tick(unsigned int now_ms);
unsigned int edge_tune_poll_ms(void);


#endif
