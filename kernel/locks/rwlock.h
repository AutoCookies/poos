#ifndef POOS_LOCKS_RWLOCK_H
#define POOS_LOCKS_RWLOCK_H

#include "spinlock.h"

typedef struct rwlock {
    spinlock_t lock;
    u32 readers;
    u32 writer;
} rwlock_t;

void rwlock_init(rwlock_t* rw);
void read_lock(rwlock_t* rw);
void read_unlock(rwlock_t* rw);
void write_lock(rwlock_t* rw);
void write_unlock(rwlock_t* rw);

#endif
