#include "proc.h"
#include "elf32.h"
#include "task.h"
#include "../mem/pmm.h"
#include "../mem/vmm.h"
#include "../mem/mem.h"
#include "../mem/heap.h"
#include "../arch/x86/cpu.h"
#include "../sched/sched.h"

u32 pid_alloc(void);
int proc_setup_user_stack(struct proc* p, u32* out_esp);

static struct proc* g_procs;
static u32 g_kernel_cr3;

void proc_init(void) { g_procs = 0; g_kernel_cr3 = cpu_read_cr3(); }

static u32 alloc_pagedir(void) {
    u32 pd_phys = pmm_alloc_frame();
    u32* pd = (u32*)(pd_phys + KERNEL_BASE);
    mem_set(pd, 0, 4096);
    u32* kpd = (u32*)(g_kernel_cr3 + KERNEL_BASE);
    for (u32 i = 768; i < 1024; ++i) pd[i] = kpd[i];
    return pd_phys;
}

struct proc* proc_create(const char* name) {
    struct proc* p = (struct proc*)kmalloc(sizeof(struct proc), 8);
    if (!p) return 0;
    mem_set(p, 0, sizeof(*p));
    p->pid = pid_alloc();
    for (u32 i = 0; i < 31 && name[i]; ++i) p->name[i] = name[i];
    p->state = PROC_RUNNING;
    p->cr3 = alloc_pagedir();
    p->user_stack_top = USER_STACK_TOP;
    p->next = g_procs; g_procs = p;
    return p;
}

void proc_switch_address_space(struct proc* p) { cpu_write_cr3(p ? p->cr3 : g_kernel_cr3); }

static void user_start(void* arg) {
    struct task* tk = (struct task*)arg;
    extern void ring3_enter(u32 eip, u32 useresp);
    ring3_enter(tk->tf.eip, tk->tf.useresp);
    for (;;) {}
}

int proc_spawn_user_image(const char* name, const u8* image, u32 size) {
    struct proc* p = proc_create(name);
    if (!p) return -1;
    u32 entry = 0, esp = 0;
    if (elf32_load_image(p, image, size, &entry) < 0) return -1;
    if (proc_setup_user_stack(p, &esp) < 0) return -1;

    struct thread* t = kthread_create(name, user_start, 0, 0);
    if (!t) return -1;
    task_bind_user_thread(t, p, entry, esp);
    t->arg = t->task_ctx;
    return 0;
}

void proc_kill_current(int code) {
    struct task* t = task_current();
    if (t && t->owner) {
        t->owner->state = PROC_ZOMBIE;
        t->owner->exit_code = code;
    }
    kthread_exit();
}

void proc_reap_zombies(void) { (void)g_procs; }
