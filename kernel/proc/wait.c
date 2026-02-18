#include "proc.h"
#include "task.h"
#include "../sched/sched.h"

int proc_waitpid(int pid, int* status) {
    struct task* t = task_current();
    struct proc* self = t ? t->owner : 0;
    if (!self) return -1;
    for (;;) {
        struct proc* it = proc_find(0);
        while (it) {
            if (it->parent == self && it->state == PROC_ZOMBIE && (pid <= 0 || (int)it->pid == pid)) {
                if (status) *status = it->exit_code;
                int rpid = (int)it->pid;
                it->state = PROC_DEAD;
                return rpid;
            }
            it = it->next;
        }
        kthread_yield();
    }
}
