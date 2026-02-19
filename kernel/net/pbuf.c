#include "pbuf.h"
#include "../mem/heap.h"
#include "../mem/mem.h"
#include "../mm/budget.h"

#define PBUF_POOL_SMALL_COUNT 256
#define PBUF_POOL_MTU_COUNT 512
#define PBUF_POOL_SMALL_SIZE 256
#define PBUF_POOL_MTU_SIZE 1536

static pbuf_t g_small_meta[PBUF_POOL_SMALL_COUNT];
static pbuf_t g_mtu_meta[PBUF_POOL_MTU_COUNT];
static u8 g_small_data[PBUF_POOL_SMALL_COUNT][PBUF_POOL_SMALL_SIZE];
static u8 g_mtu_data[PBUF_POOL_MTU_COUNT][PBUF_POOL_MTU_SIZE];
static u8 g_small_used[PBUF_POOL_SMALL_COUNT];
static u8 g_mtu_used[PBUF_POOL_MTU_COUNT];

static pbuf_t* pbuf_pool_alloc_small(u16 len, u16 headroom) {
    for (u16 i = 0; i < PBUF_POOL_SMALL_COUNT; ++i) {
        if (g_small_used[i]) continue;
        g_small_used[i] = 1;
        pbuf_t* p = &g_small_meta[i];
        p->base = g_small_data[i];
        p->data = p->base + headroom;
        p->len = len;
        p->capacity = PBUF_POOL_SMALL_SIZE;
        p->headroom = headroom;
        p->refcnt = 1;
        p->pool_kind = 1;
        p->pool_idx = i;
        p->next = 0;
        return p;
    }
    return 0;
}

static pbuf_t* pbuf_pool_alloc_mtu(u16 len, u16 headroom) {
    for (u16 i = 0; i < PBUF_POOL_MTU_COUNT; ++i) {
        if (g_mtu_used[i]) continue;
        g_mtu_used[i] = 1;
        pbuf_t* p = &g_mtu_meta[i];
        p->base = g_mtu_data[i];
        p->data = p->base + headroom;
        p->len = len;
        p->capacity = PBUF_POOL_MTU_SIZE;
        p->headroom = headroom;
        p->refcnt = 1;
        p->pool_kind = 2;
        p->pool_idx = i;
        p->next = 0;
        return p;
    }
    return 0;
}

pbuf_t* pbuf_alloc(u16 len, u16 headroom){
    u32 cap = (u32)len + (u32)headroom;
    pbuf_t* p = 0;
    if (cap <= PBUF_POOL_SMALL_SIZE) p = pbuf_pool_alloc_small(len, headroom);
    else if (cap <= PBUF_POOL_MTU_SIZE) p = pbuf_pool_alloc_mtu(len, headroom);
    if (!p) {
        mm_budget_inc_drop_packets();
        return 0;
    }
    if (mm_budget_try_charge(MM_BUDGET_NETWORK, cap) < 0) {
        pbuf_free(p);
        mm_budget_inc_drop_packets();
        return 0;
    }
    if(len) mem_set(p->data,0,len);
    return p;
}

void pbuf_ref(pbuf_t* p){ if(p) p->refcnt++; }

void pbuf_free(pbuf_t* p){
    if(!p) return;
    if(--p->refcnt) return;
    mm_budget_uncharge(MM_BUDGET_NETWORK, p->capacity);
    if (p->pool_kind == 1 && p->pool_idx < PBUF_POOL_SMALL_COUNT) g_small_used[p->pool_idx] = 0;
    else if (p->pool_kind == 2 && p->pool_idx < PBUF_POOL_MTU_COUNT) g_mtu_used[p->pool_idx] = 0;
    else {
        kfree(p->base);
        kfree(p);
    }
}

void* pbuf_push(pbuf_t* p,u16 len){ if(!p||len>p->headroom) return 0; p->data-=len; p->len=(u16)(p->len+len); p->headroom=(u16)(p->headroom-len); return p->data; }
void* pbuf_pull(pbuf_t* p,u16 len){ if(!p||len>p->len) return 0; p->data+=len; p->len=(u16)(p->len-len); p->headroom=(u16)(p->headroom+len); return p->data; }
