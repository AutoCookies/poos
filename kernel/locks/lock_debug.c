#include "spinlock.h"

u32 lock_contention_sample(spinlock_t* lock) {
    return lock->locked;
}
