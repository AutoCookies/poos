#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

enum {
    SYS_WRITE = 1, SYS_EXIT, SYS_YIELD, SYS_SLEEP, SYS_GETPID,
    SYS_OPEN, SYS_CLOSE, SYS_READ, SYS_LSEEK, SYS_STAT, SYS_GETDENTS,
    SYS_EXECVE, SYS_WAITPID, SYS_SPAWN, SYS_FORK, SYS_PIPE, SYS_DUP2, SYS_KILL, SYS_MMAP, SYS_MUNMAP
};

struct vstat { unsigned int mode, size, type; };
struct vdirent { unsigned int ino, type; char name[32]; };

static inline int syscall0(int n){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n):"memory");return r;}
static inline int syscall1(int n,int a){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a):"memory");return r;}
static inline int syscall2(int n,int a,int b){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a),"c"(b):"memory");return r;}
static inline int syscall3(int n,int a,int b,int c){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a),"c"(b),"d"(c):"memory");return r;}

int sys_write(int fd, const void* s, int len);
int sys_exit(int code);
int sys_yield(void);
int sys_sleep(int ms);
int sys_getpid(void);
int sys_open(const char* path, int flags);
int sys_close(int fd);
int sys_read(int fd, void* buf, int len);
int sys_lseek(int fd, int off, int whence);
int sys_stat(const char* path, struct vstat* st);
int sys_getdents(int fd, struct vdirent* d, int len);
int sys_execve(const char* path);
int sys_waitpid(int pid, int* status);
int sys_spawn(const char* path);
int sys_fork(void);
int sys_pipe(int fds[2]);
int sys_dup2(int oldfd,int newfd);
int sys_kill(int pid,int sig);
int sys_mmap(void* addr, int len, int prot, int flags, int fd, int off);
int sys_munmap(void* addr, int len);

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x100
#define O_TRUNC 0x200

#endif

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define MAP_PRIVATE 0x01
#define MAP_SHARED 0x02
#define MAP_FIXED 0x10
#define MAP_ANON 0x20
