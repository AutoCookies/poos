#include "smp.h"
#include "acpi_madt.h"
#include "mp_table.h"
#include "apic.h"

static bool g_smp_enabled;

void smp_init(void) {
    smp_cpu_early_init();
    lapic_init_bsp();
    ioapic_init();
    if (acpi_madt_discover_cpus() < 0) {
        mp_table_discover_cpus();
    }
    g_smp_enabled = (smp_cpu_count() > 1U) ? true : false;
}

void smp_boot_aps(void) {
    if (!g_smp_enabled) return;
    for (u32 cpu = 1; cpu < smp_cpu_count(); ++cpu) {
        apic_send_init_sipi(cpu, 0x8U);
        smp_cpu_set_online(cpu, 1);
    }
}

bool smp_enabled(void) { return g_smp_enabled; }
