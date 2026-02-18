#include "irq.h"
#include "cpu.h"
#include "../../sched/sched.h"

void panic(const char* msg);
void vga_write(const char* s);
void vga_write_u32(u32 value);
void vga_write_hex(u32 value);

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
static volatile u32 irq_depth;

static const char* exception_messages[32] = {
    "Division by zero", "Debug", "NMI", "Breakpoint", "Overflow", "Bound range exceeded", "Invalid opcode", "Device not available",
    "Double fault", "Coprocessor segment overrun", "Invalid TSS", "Segment not present", "Stack-segment fault", "General protection fault", "Page fault", "Reserved",
    "x87 floating-point exception", "Alignment check", "Machine check", "SIMD floating-point exception", "Virtualization exception", "Control protection exception", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved", "Hypervisor injection exception", "VMM communication exception", "Security exception", "Reserved"
};

static void idt_set_gate(u8 vector, u32 handler_addr, u16 selector, u8 type_attr) {
    idt[vector].offset_low = (u16)(handler_addr & 0xFFFFU);
    idt[vector].selector = selector;
    idt[vector].zero = 0;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_high = (u16)((handler_addr >> 16) & 0xFFFFU);
}

void idt_register_handler(u8 vector, interrupt_handler_t fn) { handlers[vector] = fn; }

void idt_init(void) {
    for (u32 i = 0; i < IDT_ENTRIES; ++i) {
        handlers[i] = 0;
        idt_set_gate((u8)i, (u32)isr_stub_table[i], GDT_KERNEL_CODE_SELECTOR, 0x8EU);
    }
    lidt(idt, sizeof(idt) - 1U);
}

u32 irq_nesting_depth(void) { return irq_depth; }

static void dump_page_fault(struct regs* r) {
    vga_write("Page fault addr="); vga_write_hex(cpu_read_cr2());
    vga_write(" err="); vga_write_hex(r->err_code);
    vga_write(" eip="); vga_write_hex(r->eip); vga_write("\n");
}

void isr_dispatch(struct regs* r) {
    if (r->int_no < 32U) {
        vga_write("\n[EXCEPTION] #"); vga_write_u32(r->int_no); vga_write(": ");
        vga_write(exception_messages[r->int_no]); vga_write("\n");
        if (r->int_no == 14U) { dump_page_fault(r); }
        panic("Unhandled CPU exception");
    }

    if (r->int_no >= 32U && r->int_no <= 47U) {
        ++irq_depth;
    }

    if (handlers[r->int_no] != 0) {
        handlers[r->int_no](r);
    }

    if (r->int_no >= 32U && r->int_no <= 47U) {
        pic_send_eoi((u8)(r->int_no - 32U));
        if (r->int_no == 32U && sched_need_resched() != 0U) {
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
