#include "../arch/x86/smp/cpu.h"

void time_smp_tick(void) {
    smp_cpu_tick();
}
