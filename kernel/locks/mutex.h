#ifndef POOS_LOCKS_MUTEX_H
#define POOS_LOCKS_MUTEX_H

#include "spinlock.h"

typedef struct mutex {
    spinlock_t lock;
    u32 held;
} mutex_t;

void mutex_init(mutex_t* m);
void mutex_lock(mutex_t* m);
void mutex_unlock(mutex_t* m);

#endif
