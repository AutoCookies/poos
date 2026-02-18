#include "types.h"

struct regs;

void vga_write(const char* s);
void vga_write_u32(u32 value);
void idt_register_handler(u8 vector, void (*fn)(struct regs* r));

static volatile u32 ticks = 0;

static void timer_irq(struct regs* r) {
    (void)r;
    ++ticks;

    if ((ticks % 100U) == 0U) {
        vga_write("[timer] seconds=");
        vga_write_u32(ticks / 100U);
        vga_write(" ticks=");
        vga_write_u32(ticks);
        vga_write("\n");
    }
}

void timer_init(void) {
    const u32 divisor = PIT_INPUT_HZ / 100U;

    idt_register_handler(32U, timer_irq);

    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (u8)(divisor & 0xFFU));
    outb(PIT_CHANNEL0, (u8)((divisor >> 8) & 0xFFU));
}
