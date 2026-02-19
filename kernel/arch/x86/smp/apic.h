#ifndef POOS_ARCH_X86_APIC_H
#define POOS_ARCH_X86_APIC_H

#include "../../../types.h"

void lapic_init_bsp(void);
void lapic_init_ap(void);
void ioapic_init(void);
void apic_send_init_sipi(u32 cpu, u8 vector);
void ipi_send_resched(u32 cpu);
void ipi_send_tlb_shootdown(u32 cpu);

#endif
