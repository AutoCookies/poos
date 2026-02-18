#ifndef POOS_PROC_TASK_H
#define POOS_PROC_TASK_H

#include "../types.h"
#include "proc.h"
#include "../arch/x86/idt.h"
struct thread;

typedef enum { TASK_MODE_KERNEL = 0, TASK_MODE_USER = 1 } task_mode_t;

struct task {
    struct proc* owner;
    task_mode_t mode;
    struct trapframe tf;
    u32 kstack_top;
};

void task_bind_kernel_thread(struct thread* t);
void task_bind_user_thread(struct thread* t, struct proc* p, u32 entry, u32 useresp);
void task_on_switch(struct thread* next);
struct task* task_current(void);

#endif
