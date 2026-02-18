#include "types.h"
#include "bootinfo.h"
#include "mem/mem.h"
#include "mem/pmm.h"
#include "mem/paging.h"
#include "mem/vmm.h"
#include "mem/heap.h"
#include "arch/x86/cpu.h"
#include "arch/x86/irq.h"
#include "sched/sched.h"
#include "time/time.h"

void gdt_init(void);
void vga_init(void);
void vga_write(const char* s);
void vga_write_u32(u32 value);

extern u32 __kernel_phys_start;
extern u32 __kernel_phys_end;

static void run_mapping_smoke_test(void) {
    u32 frame = pmm_alloc_frame();
    POOS_ASSERT(frame != 0U);

    const u32 test_virt = 0xC2000000U;
    POOS_ASSERT(vmm_map_page(test_virt, frame, PAGE_RW));
    *(volatile u32*)test_virt = 0xA5A55A5AU;

    u32 translated = 0;
    POOS_ASSERT(vmm_translate(test_virt, &translated));
    POOS_ASSERT((translated & 0xFFFFF000U) == frame);

    vmm_unmap_page(test_virt);
    pmm_free_frame(frame);
}

static void worker_a(void* arg) {
    (void)arg;
    for (;;) {
        vga_write("A");
        kthread_sleep(time_ms_to_ticks(200U));
    }
}

static void worker_b(void* arg) {
    (void)arg;
    for (;;) {
        vga_write("B");
        kthread_sleep(time_ms_to_ticks(350U));
    }
}

static void ticker(void* arg) {
    (void)arg;
    for (;;) {
        kthread_sleep(time_ms_to_ticks(1000U));
        vga_write(" ticks=");
        vga_write_u32(time_ticks());
        vga_write("\n");
    }
}

void kernel_main(struct BootInfo* bootinfo) {
    irq_disable();

    vga_init();
    vga_write("PoOS v0.3 booting...\n");

    gdt_init();
    irq_init();

    bootinfo->kernel_phys_start = (u32)&__kernel_phys_start;
    bootinfo->kernel_phys_end = (u32)&__kernel_phys_end;

    mem_init(bootinfo);
    mem_sanity_check();
    heap_smoke_test();
    run_mapping_smoke_test();

    time_init();
    sched_init();
    pit_init();

    kthread_create("ticker", ticker, 0, 0);
    kthread_create("workerA", worker_a, 0, 0);
    kthread_create("workerB", worker_b, 0, 0);

    vga_write("Timer Hz="); vga_write_u32(POOS_TIMER_HZ);
    vga_write(" timeslice="); vga_write_u32(SCHED_TIMESLICE_TICKS); vga_write("\n");
    sched_dump_threads();
    vga_write("Current ticks="); vga_write_u32(time_ticks()); vga_write("\n");

    irq_enable();
    sched_start();

    for (;;) {
        cpu_hlt();
    }
}
