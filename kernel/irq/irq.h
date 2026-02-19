#ifndef POOS_IRQ_CORE_H
#define POOS_IRQ_CORE_H

#include "../types.h"

void irq_core_init(void);
void irq_set_affinity(u32 irq, u32 cpu_mask);

#endif
