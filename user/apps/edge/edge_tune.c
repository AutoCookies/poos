#include "edge_internal.h"
#include "../../../kernel/mm/ram_ladder.h"
extern int printf_min(const char*, ...);

struct edge_tune_state g_edge_tune;

static const struct ram_ladder_caps g_ladder[RAM_TIER_COUNT] = {
    {64,16,8,8*1024,2*1024,1*1024*1024,8*1024*1024,3000,2*1024*1024,2*1024*1024,4*1024*1024,3},
    {80,64,32,16*1024,4*1024,4*1024*1024,16*1024*1024,12000,8*1024*1024,8*1024*1024,8*1024*1024,2},
    {96,96,48,24*1024,6*1024,8*1024*1024,32*1024*1024,14000,10*1024*1024,10*1024*1024,10*1024*1024,2},
    {128,128,64,32*1024,8*1024,16*1024*1024,64*1024*1024,18000,16*1024*1024,16*1024*1024,16*1024*1024,1},
    {160,192,96,48*1024,12*1024,24*1024*1024,96*1024*1024,22000,24*1024*1024,24*1024*1024,20*1024*1024,1},
    {200,256,128,64*1024,16*1024,32*1024*1024,128*1024*1024,30000,32*1024*1024,32*1024*1024,24*1024*1024,1},
};

static int apply_u32(int key, unsigned int v){ return sys_sysctl(1,key,&v); }

static int choose_tier(unsigned int mb){
    int tier=RAM_TIER_64MB;
    for(int i=0;i<RAM_TIER_COUNT;i++) if(mb>=g_ladder[i].tier_mb) tier=i;
    return tier;
}

static void apply_caps(const struct ram_ladder_caps* c){
    apply_u32(SYSCTL_NET_MAX_CONNS,c->net_max_conns);
    apply_u32(SYSCTL_NET_MAX_TLS_CONNS,c->net_max_tls_conns);
    apply_u32(SYSCTL_NET_TCP_BUF_CAP,c->net_tcp_buf_cap);
    apply_u32(SYSCTL_NET_TLS_RECORD_CAP,c->net_tls_record_cap);
    apply_u32(SYSCTL_PROXY_CACHE_MEM_CAP,c->proxy_cache_mem_cap);
    apply_u32(SYSCTL_PROXY_CACHE_DISK_CAP,c->proxy_cache_disk_cap);
    apply_u32(SYSCTL_PROXY_KEEPALIVE_MS,c->proxy_keepalive_ms);
    apply_u32(SYSCTL_VM_PAGE_CACHE_CAP,c->vm_page_cache_cap);
    apply_u32(SYSCTL_VM_BCACHE_CAP,c->vm_bcache_cap);
    apply_u32(SYSCTL_MM_SLAB_CAP,c->mm_slab_cap);
    apply_u32(SYSCTL_EDGE_LOG_LEVEL,c->edge_log_level);
}

int edge_tune_bootstrap(int forced_tier_mb){
    struct meminfo_u mi; if(sys_meminfo(&mi,sizeof(mi))<0){ mi.total_bytes=80u*1024u*1024u; mi.free_bytes=16u*1024u*1024u; }
    unsigned int mb=mi.total_bytes/(1024u*1024u);
    int t=choose_tier(mb);
    if(forced_tier_mb>0) t=choose_tier((unsigned int)forced_tier_mb);
    g_edge_tune.ram_total_mb=mb;
    g_edge_tune.tier=t;
    g_edge_tune.tier_mb=g_ladder[t].tier_mb;
    g_edge_tune.max_conns_cur=g_ladder[t].net_max_conns/2u;
    g_edge_tune.cooldown_until_ms=0;
    g_edge_tune.min_free_mb=mi.free_bytes/(1024u*1024u);
    apply_caps(&g_ladder[t]);
    apply_u32(SYSCTL_NET_MAX_CONNS,g_edge_tune.max_conns_cur);
    apply_u32(SYSCTL_EDGE_TIER_MB,g_edge_tune.tier_mb);
    return 0;
}

void edge_tune_tick(unsigned int now_ms){
    struct meminfo_u mi; if(sys_meminfo(&mi,sizeof(mi))<0) return;
    unsigned int free_mb=mi.free_bytes/(1024u*1024u);
    if(free_mb<g_edge_tune.min_free_mb) g_edge_tune.min_free_mb=free_mb;
    int lvl=0;
    if(free_mb<=4) lvl=3; else if(free_mb<=8) lvl=2; else if(free_mb<=12) lvl=1;
    if(lvl>g_edge_tune.pressure_level){ g_edge_tune.runtime_degrade_events++; g_edge_tune.pressure_level=lvl; }
    if(lvl==0 && g_edge_tune.pressure_level>0) g_edge_tune.pressure_level=0;

    if(lvl>=1){
        unsigned int v=(g_ladder[g_edge_tune.tier].proxy_keepalive_ms/2u); if(v<3000u) v=3000u;
        apply_u32(SYSCTL_PROXY_KEEPALIVE_MS,v);
        apply_u32(SYSCTL_VM_PAGE_CACHE_CAP,g_ladder[g_edge_tune.tier].vm_page_cache_cap/2u);
        apply_u32(SYSCTL_VM_BCACHE_CAP,g_ladder[g_edge_tune.tier].vm_bcache_cap/2u);
    } else {
        apply_u32(SYSCTL_PROXY_KEEPALIVE_MS,g_ladder[g_edge_tune.tier].proxy_keepalive_ms);
        apply_u32(SYSCTL_VM_PAGE_CACHE_CAP,g_ladder[g_edge_tune.tier].vm_page_cache_cap);
        apply_u32(SYSCTL_VM_BCACHE_CAP,g_ladder[g_edge_tune.tier].vm_bcache_cap);
    }
    { unsigned int v=(lvl>=2)?1u:0u; apply_u32(SYSCTL_PROXY_DISABLE_KEEPALIVE,v); apply_u32(SYSCTL_PROXY_DISABLE_NEW_CACHE,v); apply_u32(SYSCTL_EDGE_REFUSE_NEW_TLS,v); }
    { unsigned int v=(lvl>=3)?1u:0u; apply_u32(SYSCTL_PROXY_SHED_LOAD,v); if(v) g_edge_tune.shed_load_count++; }

    if(now_ms < g_edge_tune.cooldown_until_ms) return;
    if(lvl>=2){
        if(g_edge_tune.max_conns_cur>16u) g_edge_tune.max_conns_cur/=2u;
        if(g_edge_tune.max_conns_cur<16u) g_edge_tune.max_conns_cur=16u;
        apply_u32(SYSCTL_NET_MAX_CONNS,g_edge_tune.max_conns_cur);
    apply_u32(SYSCTL_EDGE_TIER_MB,g_edge_tune.tier_mb);
        g_edge_tune.cooldown_until_ms=now_ms+1000u;
    } else if(lvl==0) {
        unsigned int cap=g_ladder[g_edge_tune.tier].net_max_conns;
        if(g_edge_tune.max_conns_cur<cap){
            g_edge_tune.max_conns_cur+=8u; if(g_edge_tune.max_conns_cur>cap) g_edge_tune.max_conns_cur=cap;
            apply_u32(SYSCTL_NET_MAX_CONNS,g_edge_tune.max_conns_cur);
    apply_u32(SYSCTL_EDGE_TIER_MB,g_edge_tune.tier_mb);
        }
        g_edge_tune.cooldown_until_ms=now_ms+3000u;
    }
}

unsigned int edge_tune_poll_ms(void){ return g_edge_tune.tier_mb<=80?250u:1000u; }
const struct ram_ladder_caps* edge_tune_caps(void){ return &g_ladder[g_edge_tune.tier]; }
