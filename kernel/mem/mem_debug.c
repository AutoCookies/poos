#include "mem.h"
#include "e820.h"
#include "pmm.h"
#include "paging.h"

void vga_write(const char* s);

void mem_print_summary(void) {
    e820_print_summary((const struct BootInfo*)BOOTINFO_PHYS_ADDR);
    struct PmmStats stats = pmm_get_stats();
    vga_write("PMM frames total=");
    extern void vga_write_u32(u32 value);
    vga_write_u32(stats.total_frames);
    vga_write(" used=");
    vga_write_u32(stats.used_frames);
    vga_write(" free=");
    vga_write_u32(stats.free_frames);
    vga_write("\n");
    paging_print_status();
}
