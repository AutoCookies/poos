#include "irq.h"
#include "../../types.h"
#include "../../time/time.h"
#include "../../sched/sched.h"

static void pit_timer_irq(regs_t* r) {
    (void)r;
    time_tick();
    sched_on_tick_wake();
    sched_tick_from_irq();
}

void pit_init(void) {
    const u32 divisor = PIT_INPUT_HZ / POOS_TIMER_HZ;
    idt_register_handler(32U, pit_timer_irq);
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, (u8)(divisor & 0xFFU));
    outb(PIT_CHANNEL0, (u8)((divisor >> 8) & 0xFFU));
}
