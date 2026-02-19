#ifndef POOS_MM_RAM_LADDER_H
#define POOS_MM_RAM_LADDER_H

#include "../types.h"

enum ram_tier_id {
    RAM_TIER_64MB = 0,
    RAM_TIER_80MB,
    RAM_TIER_96MB,
    RAM_TIER_128MB,
    RAM_TIER_160MB,
    RAM_TIER_200MB,
    RAM_TIER_COUNT
};

enum sysctl_key {
    SYSCTL_NET_MAX_CONNS = 1,
    SYSCTL_NET_MAX_TLS_CONNS,
    SYSCTL_NET_TCP_BUF_CAP,
    SYSCTL_NET_TLS_RECORD_CAP,
    SYSCTL_PROXY_CACHE_MEM_CAP,
    SYSCTL_PROXY_CACHE_DISK_CAP,
    SYSCTL_PROXY_KEEPALIVE_MS,
    SYSCTL_VM_PAGE_CACHE_CAP,
    SYSCTL_VM_BCACHE_CAP,
    SYSCTL_MM_SLAB_CAP,
    SYSCTL_EDGE_LOG_LEVEL,
    SYSCTL_EDGE_REFUSE_NEW_TLS,
    SYSCTL_PROXY_DISABLE_NEW_CACHE,
    SYSCTL_PROXY_DISABLE_KEEPALIVE,
    SYSCTL_PROXY_SHED_LOAD,
    SYSCTL_EDGE_TIER_MB
};

struct ram_ladder_caps {
    u32 tier_mb;
    u32 net_max_conns;
    u32 net_max_tls_conns;
    u32 net_tcp_buf_cap;
    u32 net_tls_record_cap;
    u32 proxy_cache_mem_cap;
    u32 proxy_cache_disk_cap;
    u32 proxy_keepalive_ms;
    u32 vm_page_cache_cap;
    u32 vm_bcache_cap;
    u32 mm_slab_cap;
    u32 edge_log_level;
};

#endif
