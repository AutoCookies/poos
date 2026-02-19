#include "tlb.h"
#include "../arch/x86/paging_hw.h"
void tlb_flush_page(u32 va) { hw_invlpg(va & ~0xFFFU); }
