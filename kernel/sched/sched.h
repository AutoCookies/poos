#ifndef POOS_SCHED_SCHED_H
#define POOS_SCHED_SCHED_H

#include "thread.h"
#include "../arch/x86/irq.h"

#define SCHED_TIMESLICE_TICKS 10U

void sched_init(void);
void sched_start(void);
void sched_tick_from_irq(void);
void sched_on_tick_wake(void);
u32 sched_need_resched(void);
void sched_reschedule_from_irq(void);
void sched_assert_invariants(void);

struct thread* kthread_create(const char* name, void (*entry)(void*), void* arg, u32 priority);
void kthread_yield(void);
void kthread_sleep(u32 ticks);
void kthread_exit(void);

struct thread* sched_current(void);
void sched_dump_threads(void);

#endif
