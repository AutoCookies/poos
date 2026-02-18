#include "tss.h"
#include "gdt.h"

struct tss_entry {
    u32 prev_tss;
    u32 esp0;
    u32 ss0;
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax, ecx, edx, ebx;
    u32 esp, ebp, esi, edi;
    u32 es, cs, ss, ds, fs, gs;
    u32 ldt;
    u16 trap;
    u16 iomap_base;
} __attribute__((packed));

extern void gdt_set_tss_descriptor(u32 base, u32 limit);
static struct tss_entry g_tss;

void tss_set_kernel_stack(u32 esp0) {
    g_tss.esp0 = esp0;
}

void tss_init(void) {
    for (u32 i = 0; i < sizeof(g_tss); ++i) {
        ((u8*)&g_tss)[i] = 0;
    }
    g_tss.ss0 = GDT_KERNEL_DATA_SELECTOR;
    g_tss.iomap_base = sizeof(g_tss);

    gdt_set_tss_descriptor((u32)&g_tss, sizeof(g_tss) - 1U);
    __asm__ __volatile__("ltr %%ax" :: "a"(GDT_TSS_SELECTOR));
}
