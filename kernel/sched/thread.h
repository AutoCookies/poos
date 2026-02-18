#ifndef POOS_SCHED_THREAD_H
#define POOS_SCHED_THREAD_H

#include "../types.h"

#define THREAD_NAME_MAX 16U
#define THREAD_STACK_SIZE (16U * 1024U)
#define THREAD_CANARY 0xD15EA5E5U

typedef enum {
    THREAD_RUNNING = 0,
    THREAD_READY,
    THREAD_BLOCKED,
    THREAD_SLEEPING,
    THREAD_ZOMBIE
} thread_state_t;

struct thread {
    u32 id;
    char name[THREAD_NAME_MAX];
    thread_state_t state;

    u8* kernel_stack_base;
    u8* kernel_stack_top;
    u32* context_sp;

    u32 timeslice_remaining;
    u32 wakeup_tick;

    struct thread* rq_prev;
    struct thread* rq_next;
    struct thread* sleep_next;

    u32 stack_canary;
    void (*entry)(void* arg);
    void* arg;
};

void thread_fill_name(struct thread* t, const char* name);

#endif
