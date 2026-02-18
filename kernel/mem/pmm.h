#ifndef POOS_MEM_PMM_H
#define POOS_MEM_PMM_H

#include "../bootinfo.h"

#define PMM_PAGE_SIZE 4096U

struct PmmStats {
    u32 total_frames;
    u32 used_frames;
    u32 free_frames;
};

void pmm_init(const struct BootInfo* bootinfo, u32 kernel_phys_start, u32 kernel_phys_end);
u32 pmm_alloc_frame(void);
void pmm_free_frame(u32 phys_addr);
bool pmm_is_frame_used(u32 phys_addr);
struct PmmStats pmm_get_stats(void);
void pmm_reserve_region(u32 start, u32 length);
void pmm_release_region(u32 start, u32 length);

#endif
