#ifndef PROXY_STATS_H
#define PROXY_STATS_H
struct proxy_runtime_stats {
    unsigned int conns_current, conns_peak;
    unsigned int tls_handshake_ok, tls_handshake_fail;
    unsigned int req_ok, req_4xx, req_5xx;
    unsigned int cache_hit, cache_miss, cache_evict;
    unsigned int rate_limited_count, timeouts_count, upstream_fail_count;
    unsigned int shed_load_count, mem_pressure_events;
};
struct proxy_runtime_stats* proxy_stats_mut(void);
void proxy_stats_dump(void);
void proxy_stats_write_file(void);
#endif
