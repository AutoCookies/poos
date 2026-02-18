#include "../arch/x86/idt.h"
#include "sys_defs.h"

int sys_write(const char* uptr, u32 len);
int sys_exit(int code);
int sys_yield(void);
int sys_sleep(u32 ms);
int sys_getpid(void);

void syscall_dispatch(struct trapframe* tf) {
    int ret = -38;
    switch (tf->eax) {
        case SYS_WRITE: ret = sys_write((const char*)tf->ebx, tf->ecx); break;
        case SYS_EXIT: ret = sys_exit((int)tf->ebx); break;
        case SYS_YIELD: ret = sys_yield(); break;
        case SYS_SLEEP: ret = sys_sleep(tf->ebx); break;
        case SYS_GETPID: ret = sys_getpid(); break;
        default: break;
    }
    tf->eax = (u32)ret;
}
