#include "initrd.h"
#include "../types.h"

static u32 g_initrd_phys;
static u32 g_initrd_size;

void initrd_set_region(u32 phys_start, u32 size) { g_initrd_phys = phys_start; g_initrd_size = size; }
const u8* initrd_data(void) { return (const u8*)(g_initrd_phys + KERNEL_VIRT_BASE); }
u32 initrd_size(void) { return g_initrd_size; }
