#ifndef POOS_SCHED_SLEEPQ_H
#define POOS_SCHED_SLEEPQ_H

#include "thread.h"

void sleepq_init(void);
void sleepq_insert(struct thread* t);
struct thread* sleepq_wake_ready(u32 now_tick);
int sleepq_contains(struct thread* t);

#endif
