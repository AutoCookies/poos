#include "mem.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"

void panic(const char* msg);
void vga_write(const char* s);

void* mem_set(void* dest, u8 value, usize count) {
    u8* out = (u8*)dest;
    for (usize i = 0; i < count; ++i) {
        out[i] = value;
    }
    return dest;
}

void* mem_copy(void* dest, const void* src, usize count) {
    u8* d = (u8*)dest;
    const u8* s = (const u8*)src;
    for (usize i = 0; i < count; ++i) {
        d[i] = s[i];
    }
    return dest;
}

int mem_cmp(const void* a, const void* b, usize count) {
    const u8* x = (const u8*)a;
    const u8* y = (const u8*)b;
    for (usize i = 0; i < count; ++i) {
        if (x[i] != y[i]) {
            return (int)x[i] - (int)y[i];
        }
    }
    return 0;
}

void mem_init(struct BootInfo* bootinfo) {
    if (bootinfo->magic != BOOTINFO_MAGIC || bootinfo->version != BOOTINFO_VERSION) {
        panic("Invalid BootInfo contract");
    }

    pmm_init(bootinfo, bootinfo->kernel_phys_start, bootinfo->kernel_phys_end);
    vmm_init();
    heap_init();
}

void mem_sanity_check(void) {
    u32 frame = pmm_alloc_frame();
    POOS_ASSERT(frame != 0U);
    POOS_ASSERT(pmm_is_frame_used(frame));
    pmm_free_frame(frame);
    POOS_ASSERT(!pmm_is_frame_used(frame));
}
