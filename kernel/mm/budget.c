#include "budget.h"
#include "../mem/mem.h"

static struct mm_budget_snapshot g_budget;

void mm_budget_init(void) {
    mem_set(&g_budget, 0, sizeof(g_budget));
#if defined(CONFIG_EDGE_80MB)
    g_budget.cat[MM_BUDGET_KERNEL_CORE].cap = 10U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_SLAB].cap = 10U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_PAGE_CACHE].cap = 8U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_BUFFER_CACHE].cap = 8U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_NETWORK].cap = 6U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_TCP].cap = 8U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_TLS].cap = 6U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_USER_RSS].cap = 20U * 1024U * 1024U;
#else
    g_budget.cat[MM_BUDGET_KERNEL_CORE].cap = 16U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_SLAB].cap = 16U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_PAGE_CACHE].cap = 16U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_BUFFER_CACHE].cap = 16U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_NETWORK].cap = 12U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_TCP].cap = 12U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_TLS].cap = 12U * 1024U * 1024U;
    g_budget.cat[MM_BUDGET_USER_RSS].cap = 64U * 1024U * 1024U;
#endif
}

int mm_budget_try_charge(enum mm_budget_cat cat, u32 bytes) {
    if (cat >= MM_BUDGET_COUNT) return -1;
    struct mm_budget_entry* e = &g_budget.cat[cat];
    if (bytes > e->cap || e->used > (e->cap - bytes)) {
        e->refused++;
        return -1;
    }
    e->used += bytes;
    if (e->used > e->peak) e->peak = e->used;
    return 0;
}

void mm_budget_uncharge(enum mm_budget_cat cat, u32 bytes) {
    if (cat >= MM_BUDGET_COUNT) return;
    struct mm_budget_entry* e = &g_budget.cat[cat];
    if (bytes >= e->used) e->used = 0;
    else e->used -= bytes;
}

void mm_budget_set_used(enum mm_budget_cat cat, u32 bytes) {
    if (cat >= MM_BUDGET_COUNT) return;
    struct mm_budget_entry* e = &g_budget.cat[cat];
    e->used = (bytes > e->cap) ? e->cap : bytes;
    if (e->used > e->peak) e->peak = e->used;
}

void mm_budget_inc_drop_packets(void) { g_budget.dropped_packets++; }
void mm_budget_inc_refused_connections(void) { g_budget.refused_connections++; }
void mm_budget_inc_oom_kills(void) { g_budget.oom_kills++; }

void mm_budget_snapshot(struct mm_budget_snapshot* out) {
    if (!out) return;
    *out = g_budget;
}
