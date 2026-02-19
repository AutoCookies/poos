#ifndef POOS_MM_BUDGET_H
#define POOS_MM_BUDGET_H
#include "../types.h"

enum mm_budget_cat {
    MM_BUDGET_KERNEL_CORE = 0,
    MM_BUDGET_SLAB,
    MM_BUDGET_PAGE_CACHE,
    MM_BUDGET_BUFFER_CACHE,
    MM_BUDGET_NETWORK,
    MM_BUDGET_TCP,
    MM_BUDGET_TLS,
    MM_BUDGET_USER_RSS,
    MM_BUDGET_COUNT
};

struct mm_budget_entry {
    u32 cap;
    u32 used;
    u32 peak;
    u32 refused;
};

struct mm_budget_snapshot {
    struct mm_budget_entry cat[MM_BUDGET_COUNT];
    u32 dropped_packets;
    u32 refused_connections;
    u32 oom_kills;
};

void mm_budget_init(void);
int mm_budget_try_charge(enum mm_budget_cat cat, u32 bytes);
void mm_budget_uncharge(enum mm_budget_cat cat, u32 bytes);
void mm_budget_set_used(enum mm_budget_cat cat, u32 bytes);
void mm_budget_set_cap(enum mm_budget_cat cat, u32 cap);
void mm_budget_inc_drop_packets(void);
void mm_budget_inc_refused_connections(void);
void mm_budget_inc_oom_kills(void);
void mm_budget_snapshot(struct mm_budget_snapshot* out);

#endif
