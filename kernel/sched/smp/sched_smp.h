#ifndef POOS_SCHED_SMP_H
#define POOS_SCHED_SMP_H

#include "../../types.h"
#include "../thread.h"

void sched_smp_init(void);
void sched_smp_on_tick(void);
void sched_smp_enqueue(struct thread* t, u32 target_cpu);
struct thread* sched_smp_pick_next(void);
u32 sched_smp_rq_len(u32 cpu);

#endif
