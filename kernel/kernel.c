#include "types.h"

void gdt_init(void);
void idt_init(void);
void pic_remap(void);
void pic_clear_masks(void);
void timer_init(void);
void vga_init(void);
void vga_write(const char* s);

void kernel_main(void) {
    vga_init();
    vga_write("PoOS v0.1 booting...\n");

    gdt_init();
    idt_init();
    pic_remap();
    pic_clear_masks();
    timer_init();

    interrupts_enable();

    vga_write("PoOS kernel initialized at 1MiB.\n");
    vga_write("PIT configured at 100Hz.\n");
    vga_write("Entering idle loop.\n");

    for (;;) {
        cpu_halt();
    }
}
