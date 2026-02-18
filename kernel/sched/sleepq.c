#include "sleepq.h"

static struct thread* sq_head;

void sleepq_init(void) { sq_head = 0; }

void sleepq_insert(struct thread* t) {
    struct thread** link = &sq_head;
    while (*link != 0 && (*link)->wakeup_tick <= t->wakeup_tick) {
        link = &(*link)->sleep_next;
    }
    t->sleep_next = *link;
    *link = t;
}

struct thread* sleepq_wake_ready(u32 now_tick) {
    struct thread* list = 0;
    struct thread** tail = &list;
    while (sq_head != 0 && sq_head->wakeup_tick <= now_tick) {
        struct thread* t = sq_head;
        sq_head = t->sleep_next;
        t->sleep_next = 0;
        *tail = t;
        tail = &t->sleep_next;
    }
    return list;
}

int sleepq_contains(struct thread* target) {
    struct thread* it = sq_head;
    while (it != 0) {
        if (it == target) { return 1; }
        it = it->sleep_next;
    }
    return 0;
}
