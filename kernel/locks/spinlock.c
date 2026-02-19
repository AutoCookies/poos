#include "spinlock.h"
#include "../arch/x86/cpu.h"
#include "../arch/x86/smp/per_cpu.h"

void spinlock_init(spinlock_t* lock) {
    lock->locked = 0;
    lock->owner_cpu = 0xFFFFFFFFU;
}

void spin_lock(spinlock_t* lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1U) != 0U) {
    }
    lock->owner_cpu = cpu_id();
}

void spin_unlock(spinlock_t* lock) {
    lock->owner_cpu = 0xFFFFFFFFU;
    __sync_lock_release(&lock->locked);
}

spinlock_guard_t spin_lock_irqsave(spinlock_t* lock) {
    spinlock_guard_t g;
    g.irq_flags = irq_save();
    spin_lock(lock);
    return g;
}

void spin_unlock_irqrestore(spinlock_t* lock, spinlock_guard_t g) {
    spin_unlock(lock);
    irq_restore(g.irq_flags);
}
