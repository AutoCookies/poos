#ifndef POOS_LOCKS_SPINLOCK_H
#define POOS_LOCKS_SPINLOCK_H

#include "../types.h"

typedef struct spinlock {
    volatile u32 locked;
    u32 owner_cpu;
} spinlock_t;

typedef struct spinlock_guard {
    u32 irq_flags;
} spinlock_guard_t;

void spinlock_init(spinlock_t* lock);
void spin_lock(spinlock_t* lock);
void spin_unlock(spinlock_t* lock);
spinlock_guard_t spin_lock_irqsave(spinlock_t* lock);
void spin_unlock_irqrestore(spinlock_t* lock, spinlock_guard_t g);

#endif
