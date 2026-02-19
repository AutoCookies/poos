#ifndef POOS_ARCH_X86_PAGING_HW_H
#define POOS_ARCH_X86_PAGING_HW_H

#include "../../types.h"
#include "cpu.h"
#include "../../mem/mem.h"

#define HW_PAGE_SIZE 4096U
#define PTE_PRESENT 0x001U
#define PTE_RW      0x002U
#define PTE_USER    0x004U
#define PTE_PWT     0x008U
#define PTE_PCD     0x010U
#define PTE_ACCESSED 0x020U
#define PTE_DIRTY   0x040U
#define PTE_PAT     0x080U
#define PTE_GLOBAL  0x100U
#define PTE_AVL0    0x200U
#define PTE_AVL1    0x400U
#define PTE_AVL2    0x800U
#define PTE_COW     PTE_AVL0

#define PTE_ADDR_MASK 0xFFFFF000U
#define PTE_FLAG_MASK 0x00000FFFU

static inline u32* hw_pagedir_virt(u32 cr3_phys) { return (u32*)(cr3_phys + KERNEL_VIRT_BASE); }
static inline u32* hw_pt_virt_from_pde(u32 pde) { return (u32*)((pde & PTE_ADDR_MASK) + KERNEL_VIRT_BASE); }
static inline u32 hw_pde_index(u32 va) { return va >> 22; }
static inline u32 hw_pte_index(u32 va) { return (va >> 12) & 0x3FFU; }
static inline void hw_invlpg(u32 va) { invlpg((void*)va); }

#endif
