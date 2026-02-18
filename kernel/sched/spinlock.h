#ifndef POOS_SCHED_SPINLOCK_H
#define POOS_SCHED_SPINLOCK_H

#include "../types.h"
#include "../arch/x86/cpu.h"

typedef struct {
    volatile u32 locked;
} spinlock_t;

typedef struct {
    u32 irq_flags;
} spinlock_guard_t;

static inline void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
}

static inline spinlock_guard_t spin_lock_irqsave(spinlock_t* lock) {
    spinlock_guard_t g;
    g.irq_flags = irq_save();
    while (__sync_lock_test_and_set(&lock->locked, 1U) != 0U) {
    }
    return g;
}

static inline void spin_unlock_irqrestore(spinlock_t* lock, spinlock_guard_t g) {
    __sync_lock_release(&lock->locked);
    irq_restore(g.irq_flags);
}

#endif
