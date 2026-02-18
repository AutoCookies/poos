#include "vmm.h"
#include "pmm.h"
#include "mem.h"

#define VMM_PT_POOL_COUNT 64U

extern u32 __kernel_phys_start;

static u32* g_page_directory = 0;
static u32 g_page_tables[VMM_PT_POOL_COUNT][PAGE_TABLE_ENTRIES] __attribute__((aligned(PAGE_SIZE)));
static bool g_page_table_used[VMM_PT_POOL_COUNT];

static u32 virt_to_phys(u32 virt) {
    return virt - KERNEL_VIRT_BASE;
}

static u32* alloc_page_table(void) {
    for (u32 i = 0; i < VMM_PT_POOL_COUNT; ++i) {
        if (!g_page_table_used[i]) {
            g_page_table_used[i] = true;
            mem_set(g_page_tables[i], 0, PAGE_SIZE);
            return g_page_tables[i];
        }
    }
    return 0;
}

static u32* get_table(u32 pde_index, bool create, u32 flags) {
    u32 pde = g_page_directory[pde_index];
    if ((pde & PAGE_PRESENT) != 0U) {
        u32 phys = pde & 0xFFFFF000U;
        u32 virt = phys + KERNEL_VIRT_BASE;
        return (u32*)virt;
    }

    if (!create) {
        return 0;
    }

    u32* table = alloc_page_table();
    POOS_ASSERT(table != 0);
    u32 table_phys = virt_to_phys((u32)table);
    g_page_directory[pde_index] = table_phys | (flags & 0xFFFU) | PAGE_PRESENT;
    return table;
}

void vmm_init(void) {
    g_page_directory = (u32*)(read_cr3() + KERNEL_VIRT_BASE);
    mem_set(g_page_table_used, 0, sizeof(g_page_table_used));

    /* Seed pool usage for current identity + higher-half first table by address match. */
    for (u32 i = 0; i < VMM_PT_POOL_COUNT; ++i) {
        u32 phys = virt_to_phys((u32)g_page_tables[i]);
        if (((g_page_directory[0] & 0xFFFFF000U) == phys) ||
            ((g_page_directory[KERNEL_PDE_INDEX] & 0xFFFFF000U) == phys)) {
            g_page_table_used[i] = true;
        }
    }
}

bool vmm_map_page(u32 virt_addr, u32 phys_addr, u32 flags) {
    POOS_ASSERT((virt_addr % PAGE_SIZE) == 0U);
    POOS_ASSERT((phys_addr % PAGE_SIZE) == 0U);

    u32 pde_index = virt_addr >> 22;
    u32 pte_index = (virt_addr >> 12) & 0x3FFU;
    u32* table = get_table(pde_index, true, flags);
    if (table == 0) {
        return false;
    }
    if ((table[pte_index] & PAGE_PRESENT) != 0U) {
        return false;
    }
    table[pte_index] = (phys_addr & 0xFFFFF000U) | (flags & 0xFFFU) | PAGE_PRESENT;
    invlpg((void*)virt_addr);
    return true;
}

void vmm_unmap_page(u32 virt_addr) {
    POOS_ASSERT((virt_addr % PAGE_SIZE) == 0U);
    u32 pde_index = virt_addr >> 22;
    u32 pte_index = (virt_addr >> 12) & 0x3FFU;
    u32* table = get_table(pde_index, false, 0);
    if (table == 0) {
        return;
    }
    table[pte_index] = 0;
    invlpg((void*)virt_addr);
}

bool vmm_translate(u32 virt_addr, u32* out_phys) {
    u32 pde_index = virt_addr >> 22;
    u32 pte_index = (virt_addr >> 12) & 0x3FFU;
    u32* table = get_table(pde_index, false, 0);
    if (table == 0) {
        return false;
    }
    u32 pte = table[pte_index];
    if ((pte & PAGE_PRESENT) == 0U) {
        return false;
    }
    *out_phys = (pte & 0xFFFFF000U) | (virt_addr & 0xFFFU);
    return true;
}

bool vmm_map_range(u32 virt_addr, u32 phys_addr, u32 size, u32 flags) {
    u32 pages = (size + PAGE_SIZE - 1U) / PAGE_SIZE;
    for (u32 i = 0; i < pages; ++i) {
        if (!vmm_map_page(virt_addr + i * PAGE_SIZE, phys_addr + i * PAGE_SIZE, flags)) {
            return false;
        }
    }
    return true;
}
