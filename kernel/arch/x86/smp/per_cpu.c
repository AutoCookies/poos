#include "per_cpu.h"

u32 cpu_id(void) {
    return smp_cpu_id();
}
