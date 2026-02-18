#include "types.h"
#include "bootinfo.h"
#include "mem/mem.h"
#include "mem/pmm.h"
#include "mem/paging.h"
#include "mem/vmm.h"
#include "mem/heap.h"
#include "arch/x86/cpu.h"
#include "arch/x86/irq.h"
#include "arch/x86/gdt.h"
#include "sched/sched.h"
#include "time/time.h"
#include "proc/proc.h"
#include "syscall/syscall.h"

void vga_init(void);
void vga_write(const char* s);
void vga_write_u32(u32 value);

extern u32 __kernel_phys_start;
extern u32 __kernel_phys_end;

#define USER_HELLO_LOAD_PHYS 0x0012C000U
#define USER_HELLO_MAX_SIZE  0x00008000U
#define USER_FAULT_LOAD_PHYS 0x00134000U
#define USER_FAULT_MAX_SIZE  0x00004000U

static void ticker(void* arg) {
    (void)arg;
    for (;;) {
        kthread_sleep(time_ms_to_ticks(1000U));
        vga_write(" ticks=");
        vga_write_u32(time_ticks());
        vga_write("\n");
        proc_reap_zombies();
    }
}

void kernel_main(struct BootInfo* bootinfo) {
    irq_disable();

    vga_init();
    vga_write("PoOS v0.4 booting...\n");

    gdt_init();
    irq_init();

    bootinfo->kernel_phys_start = (u32)&__kernel_phys_start;
    bootinfo->kernel_phys_end = (u32)&__kernel_phys_end;

    mem_init(bootinfo);
    mem_sanity_check();
    heap_smoke_test();

    time_init();
    sched_init();
    proc_init();
    syscall_init();
    pit_init();

    kthread_create("ticker", ticker, 0, 0);

    vga_write("creating user process: /user/apps/hello\n");
    proc_spawn_user_image("hello", (const u8*)(USER_HELLO_LOAD_PHYS + KERNEL_VIRT_BASE), USER_HELLO_MAX_SIZE);
    vga_write("creating user process: /user/apps/fault\n");
    proc_spawn_user_image("fault", (const u8*)(USER_FAULT_LOAD_PHYS + KERNEL_VIRT_BASE), USER_FAULT_MAX_SIZE);

    irq_enable();
    sched_start();

    for (;;) cpu_hlt();
}
