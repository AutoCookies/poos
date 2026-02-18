#ifndef POOS_MEM_PAGING_H
#define POOS_MEM_PAGING_H

#include "../types.h"

#define PAGE_SIZE             4096U
#define PAGE_PRESENT          0x001U
#define PAGE_RW               0x002U
#define PAGE_USER             0x004U
#define PAGE_WRITE_THROUGH    0x008U
#define PAGE_CACHE_DISABLE    0x010U
#define PAGE_GLOBAL           0x100U

#define PAGE_DIRECTORY_ENTRIES 1024U
#define PAGE_TABLE_ENTRIES     1024U
#define KERNEL_PDE_INDEX       (KERNEL_VIRT_BASE >> 22)

void paging_bootstrap_init(void);
void paging_print_status(void);

#endif
