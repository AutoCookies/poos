#include "sys_defs.h"
#include "../proc/usercopy.h"
#include "../proc/task.h"
#include "../proc/proc.h"
#include "../sched/sched.h"
#include "../time/time.h"

void vga_write(const char*);

int sys_write(const char* uptr, u32 len) {
    char buf[128];
    if (len >= sizeof(buf)) len = sizeof(buf) - 1U;
    if (copy_from_user(buf, uptr, len) < 0) return -1;
    buf[len] = '\0';
    vga_write(buf);
    return (int)len;
}
int sys_exit(int code) { proc_kill_current(code); return 0; }
int sys_yield(void) { kthread_yield(); return 0; }
int sys_sleep(u32 ms) { kthread_sleep(time_ms_to_ticks(ms)); return 0; }
int sys_getpid(void) { struct task* t = task_current(); return (t && t->owner) ? (int)t->owner->pid : 0; }
