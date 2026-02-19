#include "mutex.h"
#include "../sched/sched.h"

void mutex_init(mutex_t* m) {
    spinlock_init(&m->lock);
    m->held = 0;
}

void mutex_lock(mutex_t* m) {
    for (;;) {
        spin_lock(&m->lock);
        if (m->held == 0U) {
            m->held = 1U;
            spin_unlock(&m->lock);
            return;
        }
        spin_unlock(&m->lock);
        kthread_yield();
    }
}

void mutex_unlock(mutex_t* m) {
    spin_lock(&m->lock);
    m->held = 0;
    spin_unlock(&m->lock);
}
