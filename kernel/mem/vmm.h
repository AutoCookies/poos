#ifndef POOS_MEM_VMM_H
#define POOS_MEM_VMM_H

#include "paging.h"

void vmm_init(void);
bool vmm_map_page(u32 virt_addr, u32 phys_addr, u32 flags);
void vmm_unmap_page(u32 virt_addr);
bool vmm_translate(u32 virt_addr, u32* out_phys);
bool vmm_map_range(u32 virt_addr, u32 phys_addr, u32 size, u32 flags);
bool vmm_map_page_in(u32 cr3_phys, u32 virt_addr, u32 phys_addr, u32 flags);
bool vmm_user_accessible(u32 virt_addr);

#endif
