#include "sched.h"
#include "runqueue.h"
#include "sleepq.h"
#include "spinlock.h"
#include "../mem/heap.h"
#include "../time/time.h"
#include "../arch/x86/cpu.h"
#include "../proc/task.h"

void panic(const char* msg);
void vga_write(const char* s);
void vga_write_u32(u32 value);
void vga_write_hex(u32 value);

extern void context_switch(u32** prev_sp_out, u32* next_sp);

#define MAX_THREADS 32U

static struct thread threads[MAX_THREADS];
static u32 thread_count;
static u32 next_tid = 1;
static spinlock_t sched_lock;
static struct thread* current_thread;
static struct thread* idle_thread;
static volatile u32 need_resched;

static void thread_trampoline(void);

static void sched_panic_if_bad_canary(struct thread* t) {
    if (t != 0 && t->stack_canary != THREAD_CANARY) {
        panic("thread stack canary corrupted");
    }
}

static struct thread* sched_pick_next(void) {
    struct thread* t = runqueue_pop();
    if (t == 0) {
        t = idle_thread;
    }
    return t;
}

extern void task_on_switch(struct thread* next);

static void sched_switch_to(struct thread* next) {
    struct thread* prev = current_thread;
    current_thread = next;
    next->state = THREAD_RUNNING;
    task_on_switch(next);
    next->timeslice_remaining = SCHED_TIMESLICE_TICKS;

    if (prev == 0) {
        u32* dummy = 0;
        context_switch(&dummy, next->context_sp);
    } else {
        context_switch(&prev->context_sp, next->context_sp);
    }
}

static void schedule_locked(void) {
    struct thread* prev = current_thread;
    struct thread* next = sched_pick_next();

    if (prev == next) {
        need_resched = 0;
        return;
    }

    if (prev != 0 && prev->state == THREAD_RUNNING && prev != idle_thread) {
        prev->state = THREAD_READY;
        runqueue_push(prev);
    }
    need_resched = 0;
    sched_panic_if_bad_canary(prev);
    sched_panic_if_bad_canary(next);
    sched_switch_to(next);
}

static void idle_entry(void* arg) {
    (void)arg;
    for (;;) {
        irq_enable();
        cpu_hlt();
    }
}

static void thread_trampoline(void) {
    struct thread* t = current_thread;
    irq_enable();
    t->entry(t->arg);
    kthread_exit();
}

struct thread* kthread_create(const char* name, void (*entry)(void*), void* arg, u32 priority) {
    (void)priority;
    spinlock_guard_t g = spin_lock_irqsave(&sched_lock);
    if (thread_count >= MAX_THREADS) {
        spin_unlock_irqrestore(&sched_lock, g);
        return 0;
    }

    struct thread* t = &threads[thread_count++];
    t->id = next_tid++;
    thread_fill_name(t, name);
    t->state = THREAD_READY;
    t->entry = entry;
    t->arg = arg;
    t->timeslice_remaining = SCHED_TIMESLICE_TICKS;
    t->wakeup_tick = 0;
    t->rq_next = 0; t->rq_prev = 0; t->sleep_next = 0; t->task_ctx = 0;
    t->kernel_stack_base = (u8*)kmalloc(THREAD_STACK_SIZE + 16U, 16U);
    if (t->kernel_stack_base == 0) {
        spin_unlock_irqrestore(&sched_lock, g);
        return 0;
    }
    u32 top = ((u32)t->kernel_stack_base + THREAD_STACK_SIZE + 15U) & ~0xFU;
    t->kernel_stack_top = (u8*)top;
    t->stack_canary = THREAD_CANARY;

    u32* sp = (u32*)t->kernel_stack_top;
    *(--sp) = (u32)thread_trampoline;
    *(--sp) = 0x202U;
    *(--sp) = 0U;
    *(--sp) = 0U;
    *(--sp) = 0U;
    *(--sp) = 0U;
    t->context_sp = sp;

    task_bind_kernel_thread(t);
    runqueue_push(t);
    spin_unlock_irqrestore(&sched_lock, g);
    return t;
}

void kthread_yield(void) {
    spinlock_guard_t g = spin_lock_irqsave(&sched_lock);
    need_resched = 1;
    schedule_locked();
    spin_unlock_irqrestore(&sched_lock, g);
}

void kthread_sleep(u32 ticks) {
    spinlock_guard_t g = spin_lock_irqsave(&sched_lock);
    current_thread->state = THREAD_SLEEPING;
    current_thread->wakeup_tick = time_ticks() + ticks;
    sleepq_insert(current_thread);
    need_resched = 1;
    schedule_locked();
    spin_unlock_irqrestore(&sched_lock, g);
}

void kthread_exit(void) {
    spinlock_guard_t g = spin_lock_irqsave(&sched_lock);
    current_thread->state = THREAD_ZOMBIE;
    need_resched = 1;
    schedule_locked();
    spin_unlock_irqrestore(&sched_lock, g);
    panic("returned from kthread_exit");
}

void sched_tick_from_irq(void) {
    if (current_thread == 0) {
        return;
    }
    sched_panic_if_bad_canary(current_thread);
    if (current_thread->timeslice_remaining > 0) {
        current_thread->timeslice_remaining--;
    }
    if (current_thread->timeslice_remaining == 0) {
        need_resched = 1;
    }
}

void sched_reschedule_from_irq(void) {
    if (irq_nesting_depth() > 1U) {
        panic("PANIC_ON_IRQ_NESTING_BUG");
    }
    if (need_resched == 0) {
        return;
    }
    spinlock_guard_t g = spin_lock_irqsave(&sched_lock);
    schedule_locked();
    spin_unlock_irqrestore(&sched_lock, g);
}

void sched_assert_invariants(void) {
    if (current_thread != 0 && current_thread->state != THREAD_RUNNING) {
        panic("current thread not RUNNING");
    }
    for (u32 i = 0; i < thread_count; ++i) {
        struct thread* t = &threads[i];
        if (runqueue_contains(t) && t->state == THREAD_SLEEPING) {
            panic("sleeping thread in runqueue");
        }
        if (sleepq_contains(t) && t->state != THREAD_SLEEPING) {
            panic("non-sleeping thread in sleepq");
        }
    }
}

void sched_init(void) {
    spinlock_init(&sched_lock);
    runqueue_init();
    sleepq_init();
    current_thread = 0;
    thread_count = 0;

    idle_thread = kthread_create("idle", idle_entry, 0, 0);
    if (idle_thread == 0) {
        panic("idle thread create failed");
    }
}

void sched_start(void) {
    spinlock_guard_t g = spin_lock_irqsave(&sched_lock);
    schedule_locked();
    spin_unlock_irqrestore(&sched_lock, g);
}

struct thread* sched_current(void) {
    return current_thread;
}

void sched_dump_threads(void) {
    vga_write("Threads:\n");
    for (u32 i = 0; i < thread_count; ++i) {
        struct thread* t = &threads[i];
        vga_write("  id="); vga_write_u32(t->id);
        vga_write(" name="); vga_write(t->name);
        vga_write(" stack=["); vga_write_hex((u32)t->kernel_stack_base);
        vga_write(", "); vga_write_hex((u32)t->kernel_stack_top); vga_write("]\n");
    }
}

void sched_on_tick_wake(void) {
    struct thread* list = sleepq_wake_ready(time_ticks());
    while (list != 0) {
        struct thread* t = list;
        list = list->sleep_next;
        t->sleep_next = 0;
        t->state = THREAD_READY;
        runqueue_push(t);
    }
}

u32 sched_need_resched(void) { return need_resched; }
