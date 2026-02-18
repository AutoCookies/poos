#ifndef POOS_BOOTINFO_H
#define POOS_BOOTINFO_H

#include "types.h"

#define BOOTINFO_MAGIC 0x534F4F50U /* 'POOS' little-endian */
#define BOOTINFO_VERSION 2U
#define BOOTINFO_MAX_E820_ENTRIES 128U
#define BOOTINFO_PHYS_ADDR 0x00009000U

#define BOOTINFO_FLAG_E820_VALID (1U << 0)

struct E820Entry {
    u64 base;
    u64 length;
    u32 type;
    u32 acpi_ext;
} __attribute__((packed));

struct BootInfo {
    u32 magic;
    u32 version;
    u32 flags;
    u32 e820_count;
    u32 kernel_phys_start;
    u32 kernel_phys_end;
    struct E820Entry e820_entries[BOOTINFO_MAX_E820_ENTRIES];
} __attribute__((packed));

#endif
