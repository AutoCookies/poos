#ifndef POOS_ARCH_X86_IRQ_H
#define POOS_ARCH_X86_IRQ_H

#include "../../types.h"

struct regs {
    u32 gs, fs, es, ds;
    u32 edi, esi, ebp, esp;
    u32 ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags;
};

typedef void (*interrupt_handler_t)(struct regs* r);

void idt_init(void);
void idt_register_handler(u8 vector, interrupt_handler_t fn);
void irq_init(void);
void pic_remap(void);
void pic_clear_masks(void);
void pit_init(void);
void pic_send_eoi(u8 irq);

u32 irq_nesting_depth(void);

#endif
