#include "proxy_cache.h"

#define RL_MAX 256
struct rl_entry { unsigned int ip, tokens, last_s, used; };
static struct rl_entry g_rl[RL_MAX];

int proxy_ratelimit_allow(unsigned int ip, unsigned int now_s, unsigned int rps, unsigned int burst){
    int slot=-1;
    for(int i=0;i<RL_MAX;i++){
        if(g_rl[i].used && g_rl[i].ip==ip){ slot=i; break; }
        if(!g_rl[i].used && slot<0) slot=i;
    }
    if(slot<0) slot=(int)(ip%RL_MAX);
    struct rl_entry* e=&g_rl[slot];
    if(!e->used || e->ip!=ip){ e->used=1; e->ip=ip; e->tokens=burst; e->last_s=now_s; }
    if(now_s>e->last_s){ unsigned int add=(now_s-e->last_s)*rps; e->tokens = (e->tokens+add>burst)?burst:e->tokens+add; e->last_s=now_s; }
    if(e->tokens==0) return 0;
    e->tokens--;
    return 1;
}
