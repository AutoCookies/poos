#ifndef POOS_ARCH_X86_TSS_H
#define POOS_ARCH_X86_TSS_H

#include "../../types.h"

void tss_init(void);
void tss_set_kernel_stack(u32 esp0);

#endif
