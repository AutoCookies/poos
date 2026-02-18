#include "types.h"

void vga_write(const char* s);

void panic(const char* msg) {
    interrupts_disable();
    vga_write("\n[PANIC] ");
    vga_write(msg);
    vga_write("\nSystem halted.\n");

    for (;;) {
        cpu_halt();
    }
}
