#include "time.h"

static volatile u32 g_ticks;
static volatile u32 g_epoch;
static volatile u32 g_subticks;

void time_init(void) { g_ticks = 0; g_epoch = 1735689600U; g_subticks = 0; }
void time_tick(void) { ++g_ticks; if (++g_subticks >= POOS_TIMER_HZ) { g_subticks = 0; ++g_epoch; } }
void time_tick_second(void) { ++g_epoch; }
u32 time_ticks(void) { return g_ticks; }

u32 time_ms_to_ticks(u32 ms) {
    return (ms * POOS_TIMER_HZ + 999U) / 1000U;
}

u32 time_epoch(void) { return g_epoch; }
void time_set_epoch(u32 epoch) { g_epoch = epoch; }
