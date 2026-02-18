#ifndef POOS_ARCH_X86_CPU_H
#define POOS_ARCH_X86_CPU_H

#include "../../types.h"

static inline void irq_enable(void) { __asm__ __volatile__("sti"); }
static inline void irq_disable(void) { __asm__ __volatile__("cli"); }
static inline u32 irq_save(void) {
    u32 flags;
    __asm__ __volatile__("pushf; pop %0; cli" : "=r"(flags) :: "memory");
    return flags;
}
static inline void irq_restore(u32 flags) {
    __asm__ __volatile__("push %0; popf" :: "r"(flags) : "memory", "cc");
}
static inline void cpu_hlt(void) { __asm__ __volatile__("hlt"); }

static inline u32 cpu_read_cr0(void) { u32 v; __asm__ __volatile__("mov %%cr0,%0":"=r"(v)); return v; }
static inline u32 cpu_read_cr2(void) { u32 v; __asm__ __volatile__("mov %%cr2,%0":"=r"(v)); return v; }
static inline u32 cpu_read_cr3(void) { u32 v; __asm__ __volatile__("mov %%cr3,%0":"=r"(v)); return v; }
static inline void cpu_write_cr0(u32 v) { __asm__ __volatile__("mov %0,%%cr0"::"r"(v)); }
static inline void cpu_write_cr3(u32 v) { __asm__ __volatile__("mov %0,%%cr3"::"r"(v)); }

#endif
