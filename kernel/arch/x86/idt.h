#ifndef POOS_ARCH_X86_IDT_H
#define POOS_ARCH_X86_IDT_H

#include "../../types.h"

struct trapframe {
    u32 gs, fs, es, ds;
    u32 edi, esi, ebp, esp;
    u32 ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
};

typedef void (*interrupt_handler_t)(struct trapframe* tf);

void idt_init(void);
void idt_register_handler(u8 vector, interrupt_handler_t fn);

#endif
