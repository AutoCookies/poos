#ifndef POOS_SCHED_RUNQUEUE_H
#define POOS_SCHED_RUNQUEUE_H

#include "thread.h"

void runqueue_init(void);
void runqueue_push(struct thread* t);
struct thread* runqueue_pop(void);
int runqueue_contains(struct thread* t);

#endif
