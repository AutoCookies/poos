#ifndef POOS_ARCH_X86_SMP_CPU_H
#define POOS_ARCH_X86_SMP_CPU_H

#include "../../../types.h"

#define SMP_MAX_CPUS 8U

typedef struct smp_cpu {
    u32 cpu_id;
    u32 apic_id;
    u32 online;
    u32 ticks;
    void* current;
    u32 irq_nesting;
    u32 preempt_disable;
} smp_cpu_t;

void smp_cpu_early_init(void);
void smp_cpu_set_online(u32 cpu, u32 online);
void smp_cpu_set_current(void* cur);
void* smp_cpu_current(void);
void smp_cpu_tick(void);
void smp_cpu_irq_enter(void);
void smp_cpu_irq_exit(void);
u32 smp_cpu_count(void);
u32 smp_cpu_id(void);
smp_cpu_t* smp_cpu_get(u32 cpu);

#endif
