#include "types.h"

void vga_write(const char* s);
void vga_write_u32(u32 value);

void panic(const char* msg) {
    interrupts_disable();
    vga_write("\n[PANIC] ");
    vga_write(msg);
    vga_write("\nSystem halted.\n");

    for (;;) {
        cpu_halt();
    }
}

void assertion_failed(const char* expr, const char* file, u32 line) {
    interrupts_disable();
    vga_write("\n[ASSERT] ");
    vga_write(expr);
    vga_write(" @ ");
    vga_write(file);
    vga_write(":");
    vga_write_u32(line);
    vga_write("\n");
    panic("Assertion failed");
}
