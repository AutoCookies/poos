#include "irq.h"
#include "cpu.h"
#include "../../sched/sched.h"

void pic_remap(void);
void pic_clear_masks(void);
void pic_send_eoi(u8 irq);
void faults_handle_page_fault(struct trapframe* tf);
interrupt_handler_t idt_get_handler(u8 vector);

static volatile u32 irq_depth;

u32 irq_nesting_depth(void) { return irq_depth; }

void isr_dispatch(struct trapframe* tf) {
    if (tf->int_no < 32U) {
        if (tf->int_no == 14U) {
            faults_handle_page_fault(tf);
            return;
        }
    }

    if (tf->int_no >= 32U && tf->int_no <= 47U) {
        ++irq_depth;
    }

    interrupt_handler_t h = idt_get_handler((u8)tf->int_no);
    if (h != 0) {
        h(tf);
    }

    if (tf->int_no >= 32U && tf->int_no <= 47U) {
        pic_send_eoi((u8)(tf->int_no - 32U));
        if (tf->int_no == 32U && sched_need_resched() != 0U) {
            sched_reschedule_from_irq();
        }
        --irq_depth;
    }
}

void irq_init(void) {
    irq_depth = 0;
    idt_init();
    pic_remap();
    pic_clear_masks();
}
