#include "sched_smp.h"
#include "../../arch/x86/smp/cpu.h"
#include "../../sched/runqueue.h"

void sched_smp_init(void) {}
void sched_smp_on_tick(void) {}
void sched_smp_enqueue(struct thread* t, u32 target_cpu) { (void)target_cpu; runqueue_push(t); }
struct thread* sched_smp_pick_next(void) { return runqueue_pop(); }
u32 sched_smp_rq_len(u32 cpu) { (void)cpu; return 0U; }
