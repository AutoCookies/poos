#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

static inline int syscall0(int n){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n):"memory");return r;}
static inline int syscall1(int n,int a){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a):"memory");return r;}
static inline int syscall2(int n,int a,int b){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a),"c"(b):"memory");return r;}

int sys_write(const char* s, int len);
int sys_exit(int code);
int sys_yield(void);
int sys_sleep(int ms);
int sys_getpid(void);

#endif
