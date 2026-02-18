#include "pmm.h"
#include "mem.h"

#define PMM_MAX_FRAMES (1024U * 1024U)
#define PMM_BITMAP_WORDS (PMM_MAX_FRAMES / 32U)

static u32 g_bitmap[PMM_BITMAP_WORDS];
static u32 g_total_frames;
static u32 g_used_frames;

static inline void bitmap_set(u32 frame) {
    g_bitmap[frame / 32U] |= (1U << (frame % 32U));
}

static inline void bitmap_clear(u32 frame) {
    g_bitmap[frame / 32U] &= ~(1U << (frame % 32U));
}

static inline bool bitmap_test(u32 frame) {
    return (g_bitmap[frame / 32U] & (1U << (frame % 32U))) != 0U;
}

static void mark_range(u32 start, u32 length, bool used) {
    if (length == 0U) {
        return;
    }

    u32 first = start / PMM_PAGE_SIZE;
    u32 last = (start + length - 1U) / PMM_PAGE_SIZE;
    if (last >= g_total_frames) {
        last = g_total_frames - 1U;
    }

    for (u32 frame = first; frame <= last; ++frame) {
        bool was_used = bitmap_test(frame);
        if (used && !was_used) {
            bitmap_set(frame);
            ++g_used_frames;
        } else if (!used && was_used) {
            bitmap_clear(frame);
            --g_used_frames;
        }
    }
}

void pmm_reserve_region(u32 start, u32 length) {
    mark_range(start, length, true);
}

void pmm_release_region(u32 start, u32 length) {
    mark_range(start, length, false);
}

void pmm_init(const struct BootInfo* bootinfo, u32 kernel_phys_start, u32 kernel_phys_end) {
    mem_set(g_bitmap, 0xFF, sizeof(g_bitmap));
    g_total_frames = 0;
    g_used_frames = 0;

    u64 max_addr = 0;
    for (u32 i = 0; i < bootinfo->e820_count; ++i) {
        u64 end = bootinfo->e820_entries[i].base + bootinfo->e820_entries[i].length;
        if (end > max_addr) {
            max_addr = end;
        }
    }

    g_total_frames = (u32)(max_addr / PMM_PAGE_SIZE);
    if (g_total_frames > PMM_MAX_FRAMES) {
        g_total_frames = PMM_MAX_FRAMES;
    }
    g_used_frames = g_total_frames;

    for (u32 i = 0; i < bootinfo->e820_count; ++i) {
        if (bootinfo->e820_entries[i].type != 1U) {
            continue;
        }
        u64 base = bootinfo->e820_entries[i].base;
        u64 len = bootinfo->e820_entries[i].length;
        if (base >= 0x100000000ULL) {
            continue;
        }
        if ((base + len) > 0x100000000ULL) {
            len = 0x100000000ULL - base;
        }
        pmm_release_region((u32)base, (u32)len);
    }

    pmm_reserve_region(0U, 0x00100000U);
    pmm_reserve_region(VGA_TEXT_BUFFER, 0x1000U);
    pmm_reserve_region(kernel_phys_start, kernel_phys_end - kernel_phys_start);
}

u32 pmm_alloc_frame(void) {
    for (u32 word = 0; word < (g_total_frames / 32U) + 1U; ++word) {
        if (g_bitmap[word] == 0xFFFFFFFFU) {
            continue;
        }

        for (u32 bit = 0; bit < 32U; ++bit) {
            u32 frame = word * 32U + bit;
            if (frame >= g_total_frames) {
                return 0;
            }
            if (!bitmap_test(frame)) {
                bitmap_set(frame);
                ++g_used_frames;
                return frame * PMM_PAGE_SIZE;
            }
        }
    }
    return 0;
}

void pmm_free_frame(u32 phys_addr) {
    POOS_ASSERT((phys_addr % PMM_PAGE_SIZE) == 0U);
    u32 frame = phys_addr / PMM_PAGE_SIZE;
    POOS_ASSERT(frame < g_total_frames);
    POOS_ASSERT(bitmap_test(frame));
    bitmap_clear(frame);
    --g_used_frames;
}

bool pmm_is_frame_used(u32 phys_addr) {
    POOS_ASSERT((phys_addr % PMM_PAGE_SIZE) == 0U);
    u32 frame = phys_addr / PMM_PAGE_SIZE;
    POOS_ASSERT(frame < g_total_frames);
    return bitmap_test(frame);
}

struct PmmStats pmm_get_stats(void) {
    struct PmmStats stats;
    stats.total_frames = g_total_frames;
    stats.used_frames = g_used_frames;
    stats.free_frames = g_total_frames - g_used_frames;
    return stats;
}
