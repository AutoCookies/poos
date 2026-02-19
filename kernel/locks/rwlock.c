#include "rwlock.h"
#include "../sched/sched.h"

void rwlock_init(rwlock_t* rw) {
    spinlock_init(&rw->lock);
    rw->readers = 0;
    rw->writer = 0;
}

void read_lock(rwlock_t* rw) {
    for (;;) {
        spin_lock(&rw->lock);
        if (rw->writer == 0U) {
            rw->readers++;
            spin_unlock(&rw->lock);
            return;
        }
        spin_unlock(&rw->lock);
        kthread_yield();
    }
}

void read_unlock(rwlock_t* rw) {
    spin_lock(&rw->lock);
    if (rw->readers > 0U) rw->readers--;
    spin_unlock(&rw->lock);
}

void write_lock(rwlock_t* rw) {
    for (;;) {
        spin_lock(&rw->lock);
        if (rw->writer == 0U && rw->readers == 0U) {
            rw->writer = 1U;
            spin_unlock(&rw->lock);
            return;
        }
        spin_unlock(&rw->lock);
        kthread_yield();
    }
}

void write_unlock(rwlock_t* rw) {
    spin_lock(&rw->lock);
    rw->writer = 0;
    spin_unlock(&rw->lock);
}
