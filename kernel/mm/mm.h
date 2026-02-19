#ifndef POOS_MM_MM_H
#define POOS_MM_MM_H

#include "../types.h"

struct vm_counters {
    u32 faults_total;
    u32 faults_handled;
    u32 faults_sigsegv;
    u32 cow_faults;
    u32 cow_copies;
    u32 anon_pages_alloc;
    u32 file_pages_loaded;
};

void mm_init(void);
struct vm_counters* mm_counters(void);
void mm_log_vmstat(void);
int mm_handle_page_fault(struct trapframe* tf, u32 fault_addr);

#endif
