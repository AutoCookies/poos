#ifndef USER_SYSCALL_H
#define USER_SYSCALL_H

enum {
    SYS_WRITE = 1, SYS_EXIT, SYS_YIELD, SYS_SLEEP, SYS_GETPID,
    SYS_OPEN, SYS_CLOSE, SYS_READ, SYS_LSEEK, SYS_STAT, SYS_GETDENTS,
    SYS_EXECVE, SYS_WAITPID, SYS_SPAWN, SYS_FORK, SYS_PIPE, SYS_DUP2, SYS_KILL, SYS_MMAP, SYS_MUNMAP,
    SYS_MKDIR, SYS_UNLINK, SYS_RENAME, SYS_SYNC,
    SYS_SOCKET, SYS_BIND, SYS_CONNECT, SYS_SEND, SYS_RECV, SYS_SENDTO, SYS_RECVFROM, SYS_SOCKCLOSE, SYS_NETCTL,
    SYS_GETUID, SYS_SETUID, SYS_GETEUID, SYS_CHMOD, SYS_CHOWN, SYS_UMASK, SYS_CHROOT, SYS_CAPGET, SYS_CAPSET, SYS_AUTH,
    SYS_GETRANDOM, SYS_TIME, SYS_SETTIME
};

struct vstat { unsigned int mode, size, type, uid, gid; };
struct vdirent { unsigned int ino, type; char name[32]; };
struct sockaddr_in_k { unsigned short family, port; unsigned int addr; };
struct netinfo_u { unsigned char mac[6]; unsigned int ip,mask,gw,dns,rx,tx,drops; };

static inline int syscall0(int n){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n):"memory");return r;}
static inline int syscall1(int n,int a){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a):"memory");return r;}
static inline int syscall2(int n,int a,int b){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a),"c"(b):"memory");return r;}
static inline int syscall3(int n,int a,int b,int c){int r;__asm__ volatile("int $0x80":"=a"(r):"a"(n),"b"(a),"c"(b),"d"(c):"memory");return r;}

int sys_write(int fd, const void* s, int len); int sys_exit(int code); int sys_yield(void); int sys_sleep(int ms); int sys_getpid(void);
int sys_open(const char* path, int flags); int sys_close(int fd); int sys_read(int fd, void* buf, int len); int sys_lseek(int fd, int off, int whence);
int sys_stat(const char* path, struct vstat* st); int sys_getdents(int fd, struct vdirent* d, int len); int sys_execve(const char* path); int sys_waitpid(int pid, int* status); int sys_spawn(const char* path); int sys_fork(void); int sys_pipe(int fds[2]); int sys_dup2(int oldfd,int newfd); int sys_kill(int pid,int sig); int sys_mmap(void* addr, int len, int prot, int flags, int fd, int off); int sys_munmap(void* addr, int len); int sys_mkdir(const char* path); int sys_unlink(const char* path); int sys_rename(const char* oldp, const char* newp); int sys_sync(void);
int sys_socket(int domain,int type,int proto);
int sys_bind(int fd,const struct sockaddr_in_k* sa,int len);
int sys_connect(int fd,const struct sockaddr_in_k* sa,int len);
int sys_send(int fd,const void* buf,int len,int flags);
int sys_recv(int fd,void* buf,int len,int flags);
int sys_sendto(int fd,const void* buf,int len,int flags,const struct sockaddr_in_k* sa,int alen);
int sys_recvfrom(int fd,void* buf,int len,int flags,struct sockaddr_in_k* sa,int* alen);
int sys_sockclose(int fd);
int sys_netctl(int cmd, void* buf, int len);
int sys_getuid(void); int sys_geteuid(void); int sys_setuid(int uid);
int sys_chmod(const char* path,int mode); int sys_chown(const char* path,int uid,int gid);
int sys_umask(int mask); int sys_chroot(const char* path);
int sys_capget(void); int sys_capset(int pid,int caps);
int sys_auth(const char* user,const char* pass,int* uid,int* gid);

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x100
#define O_TRUNC 0x200
#define AF_INET 2
#define SOCK_STREAM 1
#define SOCK_DGRAM 2

int sys_getrandom(void* buf, int len, int flags);
int sys_time(void);
int sys_settime(int epoch);

#endif
