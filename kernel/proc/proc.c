#include "proc.h"
#include "elf32.h"
#include "task.h"
#include "usercopy.h"
#include "../mem/pmm.h"
#include "../mem/vmm.h"
#include "../mem/mem.h"
#include "../mem/heap.h"
#include "../arch/x86/cpu.h"
#include "../sched/sched.h"
#include "../sched/thread.h"
#include "../vfs/vfs.h"
#include "../mm/addrspace.h"
#include "../mm/vma.h"
#include "../mm/cow.h"
#include "../mm/page.h"

u32 pid_alloc(void);
int proc_setup_user_stack(struct proc* p, u32* out_esp);
int proc_load_elf_from_path(struct proc* p, const char* path, u32* entry, u32* esp);

static struct proc* g_procs;
static u32 g_kernel_cr3;

extern void ring3_enter(u32 eip, u32 useresp);
extern void ring3_enter_fork(u32 eip, u32 useresp, u32 eaxret);

static void proc_setup_stdio(struct proc* p) {
    struct file* cons = 0;
    if (vfs_open("/dev/tty", 0, &cons) < 0) vfs_open("/dev/console", 0, &cons);
    if (!cons) return;
    p->fdt.files[0] = cons; file_ref(cons);
    p->fdt.files[1] = cons; file_ref(cons);
    p->fdt.files[2] = cons;
}

void proc_init(void) { g_procs = 0; g_kernel_cr3 = cpu_read_cr3(); }

struct proc* proc_find(u32 pid) {
    if (pid == 0) return g_procs;
    for (struct proc* it = g_procs; it; it = it->next) if (it->pid == pid) return it;
    return 0;
}

static u32 alloc_pagedir(void) {
    u32 pd_phys = pmm_alloc_frame();
    u32* pd = (u32*)(pd_phys + KERNEL_BASE);
    mem_set(pd, 0, 4096);
    u32* kpd = (u32*)(g_kernel_cr3 + KERNEL_BASE);
    for (u32 i = 768; i < 1024; ++i) pd[i] = kpd[i];
    return pd_phys;
}

struct proc* proc_create(const char* name, struct proc* parent) {
    struct proc* p = (struct proc*)kmalloc(sizeof(struct proc), 8);
    if (!p) return 0;
    mem_set(p, 0, sizeof(*p));
    p->pid = pid_alloc();
    for (u32 i = 0; i < 31 && name && name[i]; ++i) p->name[i] = name[i];
    p->state = PROC_RUNNING;
    p->parent = parent;
    p->ppid = parent ? parent->pid : 0;
    p->cr3 = alloc_pagedir();
    p->user_stack_top = USER_STACK_TOP;
    p->as = (struct addrspace*)kmalloc(sizeof(struct addrspace), 8);
    if (!p->as) return 0;
    addrspace_init(p->as, p->cr3);
    fdtable_init(&p->fdt);
    proc_setup_stdio(p);
    p->next = g_procs; g_procs = p;
    return p;
}

void proc_switch_address_space(struct proc* p) { cpu_write_cr3(p ? p->cr3 : g_kernel_cr3); }

static void user_start(void* arg) {
    struct task* tk = (struct task*)arg;
    ring3_enter(tk->tf.eip, tk->tf.useresp);
    for (;;) {}
}

static void user_start_fork(void* arg) {
    struct task* tk = (struct task*)arg;
    ring3_enter_fork(tk->tf.eip, tk->tf.useresp, 0);
    for (;;) {}
}

int proc_spawn_user_image(const char* name, const u8* image, u32 size) {
    struct proc* p = proc_create(name, 0); if (!p) return -1;
    u32 entry = 0, esp = 0;
    if (elf32_load_image(p, image, size, &entry) < 0) return -1;
    if (proc_setup_user_stack(p, &esp) < 0) return -1;
    struct thread* t = kthread_create(name, user_start, 0, 0); if (!t) return -1;
    task_bind_user_thread(t, p, entry, esp); t->arg = t->task_ctx;
    p->image_path = name;
    return 0;
}

int proc_spawn_path(const char* path, int* out_pid) {
    struct task* tk = task_current(); struct proc* parent = tk ? tk->owner : 0;
    struct proc* p = proc_create(path, parent); if (!p) return -1;
    if (parent) fdtable_clone(&p->fdt, &parent->fdt);
    u32 entry = 0, esp = 0;
    if (proc_load_elf_from_path(p, path, &entry, &esp) < 0) return -1;
    struct thread* t = kthread_create(path, user_start, 0, 0); if (!t) return -1;
    task_bind_user_thread(t, p, entry, esp); t->arg = t->task_ctx;
    p->image_path = path;
    if (out_pid) *out_pid = (int)p->pid;
    return 0;
}

int proc_fork_from_tf(struct trapframe* tf) {
    struct task* tk = task_current(); struct proc* parent = tk ? tk->owner : 0;
    if (!parent) return -1;
    struct proc* child = proc_create(parent->name, parent); if (!child) return -1;
    fdtable_clone(&child->fdt, &parent->fdt);
    child->image_path = parent->image_path;
    if (parent->as) { child->as->vmas = vma_clone_list(parent->as->vmas); if (!child->as->vmas && parent->as->vmas) return -1; }
    if (cow_fork_clone(child, parent) < 0) return -1;
    struct thread* t = kthread_create(child->name, user_start_fork, 0, 0); if (!t) return -1;
    task_bind_user_thread(t, child, tf->eip, tf->useresp);
    t->arg = t->task_ctx;
    return (int)child->pid;
}

void proc_child_event(struct proc* parent) { if (parent) { parent->pending_signals |= PROC_SIG_CHLD; parent->wait_gen++; } }

void proc_kill_current(int code) {
    struct task* t = task_current();
    if (t && t->owner) {
        t->owner->state = PROC_ZOMBIE;
        t->owner->exit_code = code;
        proc_child_event(t->owner->parent);
    }
    kthread_exit();
}

int proc_send_signal(u32 pid, int sig) {
    struct proc* p = proc_find(pid); if (!p) return -1;
    if (sig == SIGKILL) { p->pending_signals |= PROC_SIG_KILL; return 0; }
    if (sig == SIGSEGV) { p->pending_signals |= PROC_SIG_SEGV; return 0; }
    if (sig == SIGCHLD) { p->pending_signals |= PROC_SIG_CHLD; return 0; }
    return -1;
}
