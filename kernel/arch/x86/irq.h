#ifndef POOS_ARCH_X86_IRQ_H
#define POOS_ARCH_X86_IRQ_H

#include "idt.h"

#define PIC1_COMMAND 0x20U
#define PIC1_DATA    0x21U
#define PIC2_COMMAND 0xA0U
#define PIC2_DATA    0xA1U
#define PIC_EOI      0x20U

typedef struct trapframe regs_t;

void irq_init(void);
void pic_remap(void);
void pic_clear_masks(void);
void pit_init(void);
void pic_send_eoi(u8 irq);

u32 irq_nesting_depth(void);

#endif
