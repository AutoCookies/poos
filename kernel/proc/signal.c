#include "signal.h"
#include "task.h"

void signal_poll_current(void) {
    struct task* t = task_current();
    struct proc* p = t ? t->owner : 0;
    if (!p) return;
    if (p->pending_signals & PROC_SIG_KILL) { p->pending_signals &= ~PROC_SIG_KILL; proc_kill_current(-9); }
    if (p->pending_signals & PROC_SIG_SEGV) { p->pending_signals &= ~PROC_SIG_SEGV; proc_kill_current(-11); }
}
