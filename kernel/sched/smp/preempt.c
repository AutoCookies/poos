#include "../../arch/x86/smp/cpu.h"

void preempt_disable(void) { smp_cpu_get(smp_cpu_id())->preempt_disable++; }
void preempt_enable(void) {
    smp_cpu_t* c = smp_cpu_get(smp_cpu_id());
    if (c->preempt_disable > 0U) c->preempt_disable--;
}
