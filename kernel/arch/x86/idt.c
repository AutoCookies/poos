#include "idt.h"
#include "gdt.h"

extern void* isr_stub_table[];

struct idt_entry {
    u16 offset_low;
    u16 selector;
    u8 zero;
    u8 type_attr;
    u16 offset_high;
} __attribute__((packed));

static struct idt_entry idt[IDT_ENTRIES];
static interrupt_handler_t handlers[IDT_ENTRIES];

static void idt_set_gate(u8 vector, u32 handler_addr, u16 selector, u8 type_attr) {
    idt[vector].offset_low = (u16)(handler_addr & 0xFFFFU);
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_high = (u16)((handler_addr >> 16) & 0xFFFFU);
}

void idt_register_handler(u8 vector, interrupt_handler_t fn) { handlers[vector] = fn; }
interrupt_handler_t idt_get_handler(u8 vector) { return handlers[vector]; }

void idt_init(void) {
    for (u32 i = 0; i < IDT_ENTRIES; ++i) {
        handlers[i] = 0;
        idt_set_gate((u8)i, (u32)isr_stub_table[i], GDT_KERNEL_CODE_SELECTOR, 0x8EU);
    }
    idt_set_gate(0x80U, (u32)isr_stub_table[0x80], GDT_KERNEL_CODE_SELECTOR, 0xEEU);
    lidt(idt, sizeof(idt) - 1U);
}
