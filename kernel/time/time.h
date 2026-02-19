#ifndef POOS_TIME_H
#define POOS_TIME_H

#include "../types.h"

#define POOS_TIMER_HZ 100U

void time_init(void);
void time_tick(void);
u32 time_ticks(void);
u32 time_ms_to_ticks(u32 ms);
u32 time_epoch(void);
void time_set_epoch(u32 epoch);
void time_tick_second(void);

#endif
