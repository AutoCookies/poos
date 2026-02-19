#include "proxy_cache.h"
extern unsigned int proxy_hash_key(const char*);
extern int proxy_str_eq(const char*,const char*);
extern int proxy_slen(const char*);
extern void proxy_memcpy(void*,const void*,int);
extern void proxy_memset(void*,int,int);

static struct proxy_cache_item g_mem[PROXY_MEM_ENTRY_MAX];
static unsigned char g_arena[PROXY_MEM_CACHE_CAP];
static unsigned int g_cap = PROXY_MEM_CACHE_CAP, g_used, g_lru = 1;
static struct proxy_cache_stats g_stats;

static int arena_alloc(int n){ if(n<=0 || g_used + (unsigned int)n > g_cap) return -1; int off=(int)g_used; g_used += (unsigned int)n; return off; }

static int victim(void){ unsigned int best=0xffffffffu; int idx=-1; for(int i=0;i<PROXY_MEM_ENTRY_MAX;i++) if(g_mem[i].used && g_mem[i].lru<best){ best=g_mem[i].lru; idx=i; } return idx; }

int proxy_cache_init(unsigned int mem_cap, unsigned int disk_cap, const char* dir){ (void)disk_cap; (void)dir; if(mem_cap>0 && mem_cap<=PROXY_MEM_CACHE_CAP) g_cap=mem_cap; proxy_memset(g_mem,0,sizeof(g_mem)); g_used=0; g_lru=1; proxy_memset(&g_stats,0,sizeof(g_stats)); g_stats.mem_cap=g_cap; g_stats.disk_cap=disk_cap; return 0; }

void proxy_cache_touch_miss(void){ g_stats.miss++; }

int proxy_cache_lookup_mem(const char* key, unsigned int now, const char** hdr, int* hdr_len, const unsigned char** body, int* body_len){
    unsigned int h=proxy_hash_key(key);
    for(int i=0;i<PROXY_MEM_ENTRY_MAX;i++) if(g_mem[i].used && g_mem[i].key_hash==h && proxy_str_eq(g_mem[i].key,key)){
        if(g_mem[i].expires<=now){ g_mem[i].used=0; continue; }
        g_mem[i].lru=g_lru++; *hdr=g_mem[i].hdr; *hdr_len=g_mem[i].hdr_len; *body=g_mem[i].body; *body_len=(int)g_mem[i].body_len; g_stats.hit++; return 0;
    }
    return -1;
}

int proxy_cache_store_mem(const char* key, const char* hdr, int hdr_len, const unsigned char* body, int body_len, unsigned int ttl, unsigned int now){
    if(body_len<=0 || body_len>PROXY_SMALL_OBJ_MAX || hdr_len<=0 || hdr_len>PROXY_HDR_MAX) return -1;
    if((unsigned int)(body_len + hdr_len + 64) > g_cap) return -1;
    int idx=-1;
    for(int i=0;i<PROXY_MEM_ENTRY_MAX;i++) if(!g_mem[i].used){ idx=i; break; }
    while((idx<0) || (g_used + (unsigned int)body_len > g_cap)){
        int v=victim(); if(v<0) return -1; g_mem[v].used=0; g_stats.evictions++; if(idx<0) idx=v; if(g_used > g_mem[v].body_len) g_used -= g_mem[v].body_len; else g_used=0;
    }
    int off=arena_alloc(body_len); if(off<0) return -1;
    struct proxy_cache_item* it=&g_mem[idx]; it->used=1; it->lru=g_lru++; it->key_hash=proxy_hash_key(key); it->key_len=(unsigned short)proxy_slen(key);
    if(it->key_len>=PROXY_KEY_MAX) it->key_len=PROXY_KEY_MAX-1; proxy_memcpy(it->key,key,it->key_len); it->key[it->key_len]=0;
    it->hdr_len=(unsigned short)hdr_len; proxy_memcpy(it->hdr,hdr,hdr_len); it->body=&g_arena[off]; it->body_len=(unsigned int)body_len; proxy_memcpy(it->body,body,body_len); it->expires=now+ttl;
    g_stats.mem_used=g_used; return 0;
}

void proxy_cache_stats(struct proxy_cache_stats* out){ *out=g_stats; out->mem_used=g_used; }
