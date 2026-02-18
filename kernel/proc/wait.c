#include "proc.h"
#include "task.h"
#include "../sched/sched.h"

int proc_waitpid(int pid, int* status, int flags) {
    struct task* t = task_current();
    struct proc* self = t ? t->owner : 0;
    if (!self) return -1;
    for (;;) {
        int has_child = 0;
        for (struct proc* it = proc_find(0); it; it = it->next) {
            if (it->parent != self) continue;
            if (pid > 0 && (int)it->pid != pid) continue;
            has_child = 1;
            if (it->state == PROC_ZOMBIE) {
                if (status) *status = it->exit_code;
                int rpid = (int)it->pid;
                it->state = PROC_DEAD;
                return rpid;
            }
        }
        if (!has_child) return -10;
        if (flags & 1) return 0;
        kthread_sleep(1);
    }
}
