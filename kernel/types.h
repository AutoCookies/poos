#ifndef POOS_TYPES_H
#define POOS_TYPES_H

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed int         i32;

#define IDT_ENTRIES 256U

/* Memory layout */
#define KERNEL_PHYS_BASE 0x00100000U
#define VGA_TEXT_BUFFER  0x000B8000U

/* Segment selectors */
#define GDT_KERNEL_CODE_SELECTOR 0x08U
#define GDT_KERNEL_DATA_SELECTOR 0x10U

/* PIC ports */
#define PIC1_COMMAND 0x20U
#define PIC1_DATA    0x21U
#define PIC2_COMMAND 0xA0U
#define PIC2_DATA    0xA1U
#define PIC_EOI      0x20U

/* PIT ports */
#define PIT_COMMAND  0x43U
#define PIT_CHANNEL0 0x40U
#define PIT_INPUT_HZ 1193182U

static inline void io_wait(void) {
    __asm__ __volatile__("outb %%al, $0x80" : : "a"((u8)0));
}

static inline void outb(u16 port, u8 value) {
    __asm__ __volatile__("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port) {
    u8 value;
    __asm__ __volatile__("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void lidt(void* base, u16 size) {
    struct {
        u16 limit;
        u32 ptr;
    } __attribute__((packed)) idtr = { size, (u32)base };
    __asm__ __volatile__("lidt %0" : : "m"(idtr));
}

static inline void lgdt(void* base, u16 size) {
    struct {
        u16 limit;
        u32 ptr;
    } __attribute__((packed)) gdtr = { size, (u32)base };
    __asm__ __volatile__("lgdt %0" : : "m"(gdtr));
}

static inline void interrupts_enable(void) {
    __asm__ __volatile__("sti");
}

static inline void interrupts_disable(void) {
    __asm__ __volatile__("cli");
}

static inline void cpu_halt(void) {
    __asm__ __volatile__("hlt");
}

#endif
