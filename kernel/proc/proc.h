#ifndef POOS_PROC_PROC_H
#define POOS_PROC_PROC_H

#include "../types.h"
#include "../vfs/fdtable.h"

#define USER_BASE 0x08048000U
#define USER_STACK_TOP 0xBFFFE000U
#define KERNEL_BASE 0xC0000000U

#define SIGCHLD 17
#define SIGKILL 9
#define SIGSEGV 11

#define PROC_SIG_CHLD (1U << 0)
#define PROC_SIG_KILL (1U << 1)
#define PROC_SIG_SEGV (1U << 2)

struct trapframe;
struct addrspace;
struct cred;
struct vnode;
struct nsproxy;
struct cgroup;
struct seccomp_filter;

typedef enum { PROC_RUNNING = 0, PROC_ZOMBIE, PROC_DEAD } proc_state_t;

struct proc {
    u32 pid;
    char name[32];
    proc_state_t state;
    u32 cr3;
    u32 entry;
    u32 user_stack_top;
    int exit_code;
    int exit_reason;
    u32 wait_gen;
    u32 wait_seen;
    u32 pending_signals;
    struct proc* parent;
    u32 ppid;
    const char* image_path;
    struct fdtable fdt;
    struct addrspace* as;
    struct cred* cred;
    struct vnode* root_vnode;
    struct nsproxy* nsproxy;
    struct cgroup* cgrp;
    u32 pid_ns;
    struct seccomp_filter* seccomp;
    struct proc* next;
};

void proc_init(void);
struct proc* proc_create(const char* name, struct proc* parent);
void proc_switch_address_space(struct proc* p);
void proc_kill_current(int code);
void proc_reap_zombies(void);
int proc_spawn_user_image(const char* name, const u8* image, u32 size);
int proc_spawn_path(const char* path, int* out_pid);
int proc_exec_path_current(const char* path);
int proc_waitpid(int pid, int* status, int flags);
struct proc* proc_find(u32 pid);
int proc_setup_user_stack(struct proc* p, u32* out_esp);
int proc_fork_from_tf(struct trapframe* tf);
int proc_send_signal(u32 pid, int sig);
void proc_child_event(struct proc* parent);
struct cred* cred_current(void);
struct vnode* proc_current_root(void);
int proc_setuid(u32 uid);
int proc_setgid(u32 gid);
int proc_capset(u32 pid, u32 caps);
int proc_chroot(const char* path);
int proc_clone(u32 flags, struct trapframe* tf);
int proc_unshare(u32 flags);
int proc_setns(int fd, u32 nstype);
int proc_attach_cgroup(struct proc* p, struct cgroup* cg);

#endif
