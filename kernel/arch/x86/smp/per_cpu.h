#ifndef POOS_ARCH_X86_SMP_PER_CPU_H
#define POOS_ARCH_X86_SMP_PER_CPU_H

#include "../../../types.h"
#include "cpu.h"

#define DEFINE_PER_CPU(type, name) type per_cpu__##name[SMP_MAX_CPUS]
#define per_cpu(name, cpu) (per_cpu__##name[(cpu)])
#define this_cpu_ptr(name) (&per_cpu__##name[smp_cpu_id()])

u32 cpu_id(void);

#endif
