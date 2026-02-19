#include "cpu.h"
#include "../cpu.h"

static smp_cpu_t g_cpus[SMP_MAX_CPUS];
static volatile u32 g_cpu_count = 1;

void smp_cpu_early_init(void) {
    for (u32 i = 0; i < SMP_MAX_CPUS; ++i) {
        g_cpus[i].cpu_id = i;
        g_cpus[i].apic_id = i;
        g_cpus[i].online = (i == 0U) ? 1U : 0U;
        g_cpus[i].ticks = 0;
        g_cpus[i].current = 0;
        g_cpus[i].irq_nesting = 0;
        g_cpus[i].preempt_disable = 0;
    }
}

void smp_cpu_set_online(u32 cpu, u32 online) {
    if (cpu >= SMP_MAX_CPUS) return;
    g_cpus[cpu].online = (online != 0U) ? 1U : 0U;
    if (online != 0U && cpu + 1U > g_cpu_count) g_cpu_count = cpu + 1U;
}

void smp_cpu_set_current(void* cur) { g_cpus[smp_cpu_id()].current = cur; }
void* smp_cpu_current(void) { return g_cpus[smp_cpu_id()].current; }
void smp_cpu_tick(void) { g_cpus[smp_cpu_id()].ticks++; }
void smp_cpu_irq_enter(void) { g_cpus[smp_cpu_id()].irq_nesting++; }
void smp_cpu_irq_exit(void) { if (g_cpus[smp_cpu_id()].irq_nesting > 0U) g_cpus[smp_cpu_id()].irq_nesting--; }
u32 smp_cpu_count(void) { return g_cpu_count; }
u32 smp_cpu_id(void) { return 0U; }
smp_cpu_t* smp_cpu_get(u32 cpu) { return (cpu < SMP_MAX_CPUS) ? &g_cpus[cpu] : 0; }
