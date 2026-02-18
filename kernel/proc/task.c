#include "task.h"
#include "proc.h"
#include "../sched/thread.h"
#include "../sched/sched.h"
#include "../arch/x86/tss.h"

static struct task g_task_table[32];

struct task* task_current(void) {
    struct thread* t = sched_current();
    return t ? (struct task*)t->task_ctx : 0;
}

void task_bind_kernel_thread(struct thread* t) {
    struct task* tk = &g_task_table[t->id % 32U];
    tk->owner = 0; tk->mode = TASK_MODE_KERNEL; tk->kstack_top = (u32)t->kernel_stack_top;
    t->task_ctx = tk;
}

void task_bind_user_thread(struct thread* t, struct proc* p, u32 entry, u32 useresp) {
    struct task* tk = &g_task_table[t->id % 32U];
    tk->owner = p; tk->mode = TASK_MODE_USER; tk->kstack_top = (u32)t->kernel_stack_top;
    tk->tf.eip = entry; tk->tf.useresp = useresp;
    t->task_ctx = tk;
}

void task_on_switch(struct thread* next) {
    struct task* tk = (struct task*)next->task_ctx;
    if (!tk) return;
    tss_set_kernel_stack((u32)next->kernel_stack_top);
    proc_switch_address_space(tk->owner);
}
