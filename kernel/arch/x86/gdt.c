#include "gdt.h"
#include "tss.h"

struct gdt_entry {
    u16 limit_low;
    u16 base_low;
    u8 base_mid;
    u8 access;
    u8 granularity;
    u8 base_high;
} __attribute__((packed));

static struct gdt_entry gdt[6];

static void gdt_set_entry(u32 index, u32 base, u32 limit, u8 access, u8 granularity) {
    gdt[index].base_low = (u16)(base & 0xFFFFU);
    gdt[index].base_mid = (u8)((base >> 16) & 0xFFU);
    gdt[index].base_high = (u8)((base >> 24) & 0xFFU);
    gdt[index].limit_low = (u16)(limit & 0xFFFFU);
    gdt[index].granularity = (u8)(((limit >> 16) & 0x0FU) | (granularity & 0xF0U));
    gdt[index].access = access;
}

void gdt_set_tss_descriptor(u32 base, u32 limit) {
    gdt_set_entry(5, base, limit, 0x89U, 0x40U);
}

void gdt_init(void) {
    gdt_set_entry(0, 0, 0, 0, 0);
    gdt_set_entry(1, 0, 0xFFFFFU, 0x9AU, 0xCFU);
    gdt_set_entry(2, 0, 0xFFFFFU, 0x92U, 0xCFU);
    gdt_set_entry(3, 0, 0xFFFFFU, 0xFAU, 0xCFU);
    gdt_set_entry(4, 0, 0xFFFFFU, 0xF2U, 0xCFU);
    gdt_set_entry(5, 0, 0, 0, 0);

    lgdt(gdt, sizeof(gdt) - 1U);
    __asm__ __volatile__(
        "movw $0x10, %%ax\n"
        "movw %%ax, %%ds\n"
        "movw %%ax, %%es\n"
        "movw %%ax, %%fs\n"
        "movw %%ax, %%gs\n"
        "movw %%ax, %%ss\n"
        "ljmp $0x08, $.flush\n"
        ".flush:\n" ::: "ax", "memory");

    tss_init();
}
