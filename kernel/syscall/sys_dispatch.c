#include "../proc/signal.h"
#include "../arch/x86/idt.h"
#include "sys_defs.h"

int sys_write(int fd, const void* buf, u32 len);
int sys_exit(int code);
int sys_yield(void);
int sys_sleep(u32 ms);
int sys_getpid(void);
int sys_open(const char* path, u32 flags);
int sys_close(int fd);
int sys_read(int fd, void* buf, u32 len);
int sys_lseek(int fd, i32 off, int whence);
int sys_stat(const char* path, void* st);
int sys_getdents(int fd, void* buf, u32 len);
int sys_execve(const char* path);
int sys_waitpid(int pid, int* status);
int sys_spawn(const char* path);
int sys_fork(struct trapframe* tf);
int sys_pipe(int* ufds);
int sys_dup2(int oldfd, int newfd);
int sys_kill(int pid, int sig);

void syscall_dispatch(struct trapframe* tf) {
    int ret = -38;
    switch (tf->eax) {
        case SYS_WRITE: ret = sys_write((int)tf->ebx, (const void*)tf->ecx, tf->edx); break;
        case SYS_EXIT: ret = sys_exit((int)tf->ebx); break;
        case SYS_YIELD: ret = sys_yield(); break;
        case SYS_SLEEP: ret = sys_sleep(tf->ebx); break;
        case SYS_GETPID: ret = sys_getpid(); break;
        case SYS_OPEN: ret = sys_open((const char*)tf->ebx, tf->ecx); break;
        case SYS_CLOSE: ret = sys_close((int)tf->ebx); break;
        case SYS_READ: ret = sys_read((int)tf->ebx, (void*)tf->ecx, tf->edx); break;
        case SYS_LSEEK: ret = sys_lseek((int)tf->ebx, (i32)tf->ecx, (int)tf->edx); break;
        case SYS_STAT: ret = sys_stat((const char*)tf->ebx, (void*)tf->ecx); break;
        case SYS_GETDENTS: ret = sys_getdents((int)tf->ebx, (void*)tf->ecx, tf->edx); break;
        case SYS_EXECVE: ret = sys_execve((const char*)tf->ebx); break;
        case SYS_WAITPID: ret = sys_waitpid((int)tf->ebx, (int*)tf->ecx); break;
        case SYS_SPAWN: ret = sys_spawn((const char*)tf->ebx); break;
        case SYS_FORK: ret = sys_fork(tf); break;
        case SYS_PIPE: ret = sys_pipe((int*)tf->ebx); break;
        case SYS_DUP2: ret = sys_dup2((int)tf->ebx, (int)tf->ecx); break;
        case SYS_KILL: ret = sys_kill((int)tf->ebx, (int)tf->ecx); break;
        default: break;
    }
    tf->eax = (u32)ret;
    signal_poll_current();
}
