#include "apic.h"
#include "cpu.h"

void apic_send_init_sipi(u32 cpu, u8 vector) {
    (void)cpu;
    (void)vector;
}

void ipi_send_resched(u32 cpu) { (void)cpu; }
void ipi_send_tlb_shootdown(u32 cpu) { (void)cpu; }
