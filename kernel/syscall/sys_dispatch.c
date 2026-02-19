#include "../proc/signal.h"
#include "../arch/x86/idt.h"
#include "sys_defs.h"
#include "../mm/mmap.h"

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
int sys_mmap(void* addr, u32 len, int prot, int flags, int fd, u32 off);
int sys_munmap(void* addr, u32 len);
int sys_mkdir(const char* path);
int sys_unlink(const char* path);
int sys_rename(const char* oldp,const char* newp);
int sys_sync(void);
int sys_socket(int domain,int type,int proto);
int sys_bind(int fd,const void* sa,u32 len);
int sys_connect(int fd,const void* sa,u32 len);
int sys_send(int fd,const void* buf,u32 len,u32 flags);
int sys_recv(int fd,void* buf,u32 len,u32 flags);
int sys_sendto(int fd,const void* buf,u32 len,u32 flags,const void* sa,u32 alen);
int sys_recvfrom(int fd,void* buf,u32 len,u32 flags,void* sa,u32* alen);
int sys_sockclose(int fd);
int sys_netctl(int cmd, void* buf, u32 len);
int sys_getuid(void); int sys_setuid(int uid); int sys_geteuid(void);
int sys_chmod(const char* path,u32 mode); int sys_chown(const char* path,u32 uid,u32 gid);
int sys_umask(u32 mask); int sys_chroot(const char* path); int sys_capget(void); int sys_capset(int pid,u32 caps);
int sys_auth(const char* user,const char* pass,u32* uid,u32* gid);
int sys_getrandom(void* buf,u32 len,u32 flags);
int sys_time(void);
int sys_settime(u32 epoch);

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
        case SYS_MMAP: ret = sys_mmap((void*)tf->ebx, tf->ecx, (int)tf->edx, (int)tf->esi, (int)tf->edi, tf->ebp); break;
        case SYS_MUNMAP: ret = sys_munmap((void*)tf->ebx, tf->ecx); break;
        case SYS_MKDIR: ret = sys_mkdir((const char*)tf->ebx); break;
        case SYS_UNLINK: ret = sys_unlink((const char*)tf->ebx); break;
        case SYS_RENAME: ret = sys_rename((const char*)tf->ebx,(const char*)tf->ecx); break;
        case SYS_SYNC: ret = sys_sync(); break;
        case SYS_SOCKET: ret = sys_socket((int)tf->ebx,(int)tf->ecx,(int)tf->edx); break;
        case SYS_BIND: ret = sys_bind((int)tf->ebx,(const void*)tf->ecx,tf->edx); break;
        case SYS_CONNECT: ret = sys_connect((int)tf->ebx,(const void*)tf->ecx,tf->edx); break;
        case SYS_SEND: ret = sys_send((int)tf->ebx,(const void*)tf->ecx,tf->edx,tf->esi); break;
        case SYS_RECV: ret = sys_recv((int)tf->ebx,(void*)tf->ecx,tf->edx,tf->esi); break;
        case SYS_SENDTO: ret = sys_sendto((int)tf->ebx,(const void*)tf->ecx,tf->edx,tf->esi,(const void*)tf->edi,tf->ebp); break;
        case SYS_RECVFROM: ret = sys_recvfrom((int)tf->ebx,(void*)tf->ecx,tf->edx,tf->esi,(void*)tf->edi,(u32*)tf->ebp); break;
        case SYS_SOCKCLOSE: ret = sys_sockclose((int)tf->ebx); break;
        case SYS_NETCTL: ret = sys_netctl((int)tf->ebx,(void*)tf->ecx,tf->edx); break;
        case SYS_GETUID: ret = sys_getuid(); break;
        case SYS_SETUID: ret = sys_setuid((int)tf->ebx); break;
        case SYS_GETEUID: ret = sys_geteuid(); break;
        case SYS_CHMOD: ret = sys_chmod((const char*)tf->ebx,tf->ecx); break;
        case SYS_CHOWN: ret = sys_chown((const char*)tf->ebx,tf->ecx,tf->edx); break;
        case SYS_UMASK: ret = sys_umask(tf->ebx); break;
        case SYS_CHROOT: ret = sys_chroot((const char*)tf->ebx); break;
        case SYS_CAPGET: ret = sys_capget(); break;
        case SYS_CAPSET: ret = sys_capset((int)tf->ebx,tf->ecx); break;
        case SYS_AUTH: ret = sys_auth((const char*)tf->ebx,(const char*)tf->ecx,(u32*)tf->edx,(u32*)tf->esi); break;
        case SYS_GETRANDOM: ret = sys_getrandom((void*)tf->ebx,tf->ecx,tf->edx); break;
        case SYS_TIME: ret = sys_time(); break;
        case SYS_SETTIME: ret = sys_settime(tf->ebx); break;
        default: break;
    }
    tf->eax = (u32)ret;
    signal_poll_current();
}
