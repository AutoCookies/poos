#ifndef POOS_PROC_PROC_H
#define POOS_PROC_PROC_H

#include "../types.h"
#include "../vfs/fdtable.h"

#define USER_BASE 0x08048000U
#define USER_STACK_TOP 0xBFFFE000U
#define KERNEL_BASE 0xC0000000U

typedef enum { PROC_RUNNING = 0, PROC_ZOMBIE, PROC_DEAD } proc_state_t;

struct proc {
    u32 pid;
    char name[32];
    proc_state_t state;
    u32 cr3;
    u32 entry;
    u32 user_stack_top;
    int exit_code;
    struct proc* parent;
    struct fdtable fdt;
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
int proc_waitpid(int pid, int* status);
struct proc* proc_find(u32 pid);
int proc_setup_user_stack(struct proc* p, u32* out_esp);

#endif
