#include "runqueue.h"

static struct thread* rq_head;
static struct thread* rq_tail;

void runqueue_init(void) { rq_head = 0; rq_tail = 0; }

void runqueue_push(struct thread* t) {
    t->rq_next = 0;
    t->rq_prev = rq_tail;
    if (rq_tail != 0) { rq_tail->rq_next = t; } else { rq_head = t; }
    rq_tail = t;
}

struct thread* runqueue_pop(void) {
    struct thread* t = rq_head;
    if (t == 0) { return 0; }
    rq_head = t->rq_next;
    if (rq_head != 0) { rq_head->rq_prev = 0; } else { rq_tail = 0; }
    t->rq_prev = 0;
    t->rq_next = 0;
    return t;
}

int runqueue_contains(struct thread* target) {
    struct thread* it = rq_head;
    while (it != 0) {
        if (it == target) { return 1; }
        it = it->rq_next;
    }
    return 0;
}
