# PoOS v0.3

PoOS v0.3 adds a preemptive kernel threading and scheduling foundation on top of the v0.2 memory stack.

## Concurrency model

- **Kernel threads (`kthreads`)** with explicit lifecycle states:
  - `THREAD_RUNNING`
  - `THREAD_READY`
  - `THREAD_BLOCKED`
  - `THREAD_SLEEPING`
  - `THREAD_ZOMBIE`
- **Per-thread kernel stack**: 16 KiB stack per thread allocated from kernel heap, aligned to 16 bytes.
- **Thread control block (TCB)** stores id, name, state, stack base/top, saved context SP, timeslice bookkeeping, wakeup tick, queue links, and stack canary.

## Context switch ABI

Context switching is split between C scheduler policy and ASM state movement:

- C signature:
  - `void context_switch(u32** prev_sp_out, u32* next_sp);`
- ASM (`kernel/sched/context_switch.asm`) saves/restores:
  - `EFLAGS`, `EBX`, `ESI`, `EDI`, `EBP`, and stack pointer (`ESP` via `prev_sp_out`).
- New threads are bootstrapped with a fabricated stack frame that returns into a trampoline which invokes thread entry and then `kthread_exit()`.

## Scheduler policy (round-robin v1)

- **Preemptive round-robin** with fixed timeslice: `SCHED_TIMESLICE_TICKS = 10`.
- **Timer source**: PIT at `POOS_TIMER_HZ = 100`.
- On each timer IRQ:
  1. increment global tick
  2. wake sleeping threads whose `wakeup_tick <= now`
  3. decrement running thread timeslice
  4. request reschedule when slice reaches zero
- IRQ return path triggers `sched_reschedule_from_irq()` when reschedule is pending.

## Runqueue and sleep queue

- **Runqueue**: intrusive O(1) FIFO doubly-linked list.
- **Sleep queue**: intrusive sorted singly-linked list by wake tick.
- `kthread_sleep(ticks)` blocks without busy waiting and requeues the thread when wakeup condition is met.

## Public thread API

- `kthread_create(name, entry, arg, priority)`
- `kthread_yield()`
- `kthread_sleep(ticks)`
- `kthread_exit()`

## IRQ/preemption boundary rules

- All hardware IRQs send PIC EOI.
- Timer IRQ remains minimal and performs no slow logging.
- Scheduler checks IRQ nesting depth and panics on unsupported nested scheduling (`PANIC_ON_IRQ_NESTING_BUG`).
- Scheduling and queue mutation occur with interrupts disabled under a UP-safe spinlock.

## Diagnostics and invariants

- Thread stack canary (`THREAD_CANARY`) checked during timer tick and context switches.
- `sched_assert_invariants()` validates:
  - current thread is `RUNNING`
  - sleeping threads are not present in runqueue
  - only sleeping threads are in sleep queue
- Existing panic/page-fault reporting remains active.

## Boot demo behavior

Boot prints:

- PoOS v0.3 banner
- timer frequency and timeslice
- thread table with id/name/stack range
- current tick counter

Demo threads:

- `idle`: halts CPU in low-power idle loop
- `workerA`: prints `A` every ~200 ms
- `workerB`: prints `B` every ~350 ms
- `ticker`: prints `ticks=...` once per second

Output should interleave continuously under preemption.

## Known limitations

- UP only (single-core scheduling model)
- no user-mode processes yet
- no syscall interface yet
- no per-process address-space switching yet
- fixed in-kernel thread table (`MAX_THREADS=32`)

## Build and run

```bash
make build
make run
```
