#include "e820.h"
#include "mem.h"

void vga_write(const char* s);
void vga_write_u32(u32 value);

static void vga_write_u64_decimal(u64 value) {
    char buf[32];
    u32 idx = 0;
    if (value == 0ULL) {
        vga_write("0");
        return;
    }

    while (value > 0ULL && idx < sizeof(buf)) {
        buf[idx++] = (char)('0' + (value % 10ULL));
        value /= 10ULL;
    }

    while (idx > 0U) {
        extern void vga_putc(char c);
        vga_putc(buf[--idx]);
    }
}

void e820_print_summary(const struct BootInfo* bootinfo) {
    u64 usable = 0;
    u64 reserved = 0;

    for (u32 i = 0; i < bootinfo->e820_count; ++i) {
        if (bootinfo->e820_entries[i].type == 1U) {
            usable += bootinfo->e820_entries[i].length;
        } else {
            reserved += bootinfo->e820_entries[i].length;
        }
    }

    vga_write("E820 entries=");
    vga_write_u32(bootinfo->e820_count);
    vga_write(" usable_bytes=");
    vga_write_u64_decimal(usable);
    vga_write(" reserved_bytes=");
    vga_write_u64_decimal(reserved);
    vga_write("\n");
}
