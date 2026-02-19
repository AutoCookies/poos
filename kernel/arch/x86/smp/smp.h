#ifndef POOS_ARCH_X86_SMP_H
#define POOS_ARCH_X86_SMP_H

#include "cpu.h"

void smp_init(void);
void smp_boot_aps(void);
bool smp_enabled(void);

#endif
