#include "time.h"

static volatile u32 g_ticks;

void time_init(void) { g_ticks = 0; }
void time_tick(void) { ++g_ticks; }
u32 time_ticks(void) { return g_ticks; }

u32 time_ms_to_ticks(u32 ms) {
    return (ms * POOS_TIMER_HZ + 999U) / 1000U;
}
