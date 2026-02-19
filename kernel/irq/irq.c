#include "irq.h"

void irq_core_init(void) {}
void irq_set_affinity(u32 irq, u32 cpu_mask) { (void)irq; (void)cpu_mask; }
