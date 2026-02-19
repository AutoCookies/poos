#ifndef POOS_MM_COW_H
#define POOS_MM_COW_H

#include "../types.h"
struct proc;
int cow_fork_clone(struct proc* child, struct proc* parent);
int cow_handle_write_fault(struct proc* p, u32 fault_addr);

#endif
