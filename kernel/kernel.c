#include "types.h"
#include "bootinfo.h"
#include "mem/mem.h"
#include "mem/pmm.h"
#include "mem/paging.h"
#include "mem/vmm.h"
#include "mem/heap.h"

void gdt_init(void);
void idt_init(void);
void pic_remap(void);
void pic_clear_masks(void);
void timer_init(void);
void vga_init(void);
void vga_write(const char* s);
void vga_write_u32(u32 value);
void vga_write_hex(u32 value);

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

    vga_write("VMM mapping smoke test passed frame=");
    vga_write_hex(frame);
    vga_write("\n");
}

void kernel_main(struct BootInfo* bootinfo) {
    interrupts_disable();

    vga_init();
    vga_write("PoOS v0.2 booting...\n");

    gdt_init();
    idt_init();
    pic_remap();
    pic_clear_masks();

    bootinfo->kernel_phys_start = (u32)&__kernel_phys_start;
    bootinfo->kernel_phys_end = (u32)&__kernel_phys_end;

    mem_init(bootinfo);
    mem_sanity_check();
    heap_smoke_test();
    run_mapping_smoke_test();

    timer_init();
    interrupts_enable();

    mem_print_summary();
    vga_write("Entering idle loop.\n");

    for (;;) {
        cpu_halt();
    }
}
