# PoOS v0.2

PoOS v0.2 extends the v0.1 boot/interrupt/timer foundation with a complete memory stack suitable for future SMP and user-mode work:

- BIOS E820 memory discovery passed through a strict `BootInfo` contract.
- Bitmap-based physical memory manager (PMM) with explicit reserved regions.
- 32-bit paging with identity + higher-half kernel mapping (`0xC0000000`).
- Virtual memory management APIs for page/range map/unmap/translate.
- Deterministic kernel heap that grows by mapping PMM frames.
- Hard invariants (`POOS_ASSERT`) and memory sanity checks.

## Boot and memory contract

### BootInfo source of truth

`kernel/bootinfo.h` defines:

- `BootInfo.magic` (`'POOS'`)
- `BootInfo.version` (`2`)
- `BootInfo.flags`
- `BootInfo.e820_count`
- `BootInfo.e820_entries[128]`
- `BootInfo.kernel_phys_start/kernel_phys_end`

The boot sector populates BootInfo at physical `0x9000` and kernel entry receives a pointer to it in `EAX`.

### E820 flow

1. Boot code runs INT `0x15/E820` in real mode.
2. Entries are stored in `BootInfo.e820_entries`.
3. Kernel validates `magic/version` and uses E820 as the PMM source of truth.

## Higher-half model

PoOS now uses a higher-half linked kernel:

- Physical load base: `0x00100000`
- Virtual link base: `0xC0000000`
- Linked start VMA: `0xC0100000`

Linker exports:

- `__kernel_phys_start`, `__kernel_phys_end`
- `__kernel_virt_start`, `__kernel_virt_end`
- `__bss_start`, `__bss_end`

Early entry (`kernel/entry.asm`) builds bootstrap page structures, identity maps low 4 MiB, aliases that mapping at PDE index 768 (`0xC0000000`), enables paging, then jumps into the higher-half execution path.

## PMM design

PMM (`kernel/mem/pmm.c`) uses 4 KiB frames and a bitmap:

- Tracks up to 4 GiB (1,048,576 frames).
- Starts as fully reserved, frees only E820 `type=1` ranges.
- Re-reserves critical regions:
  - `0x00000000 - 0x000FFFFF`
  - VGA MMIO page (`0xB8000`)
  - kernel physical image range

APIs:

- `pmm_alloc_frame()`
- `pmm_free_frame()`
- `pmm_reserve_region()` / `pmm_release_region()`
- `pmm_get_stats()`

## Paging + VMM design

Page flags are centralized in `kernel/mem/paging.h`:

- `PAGE_PRESENT`, `PAGE_RW`, `PAGE_USER`, `PAGE_WRITE_THROUGH`, `PAGE_CACHE_DISABLE`, `PAGE_GLOBAL`

VMM (`kernel/mem/vmm.c`) exposes:

- `vmm_map_page(virt, phys, flags)`
- `vmm_unmap_page(virt)`
- `vmm_translate(virt, &phys)`
- `vmm_map_range(virt, phys, size, flags)`

Kernel keeps both identity and higher-half mapping active for stability during bring-up.

## Kernel heap strategy

Heap (`kernel/mem/heap.c`) is a deterministic free-list allocator in virtual region `0xC1000000+`:

- Initial mapped size: 16 pages.
- Growth: allocate frames from PMM, map into heap via VMM.
- API:
  - `kmalloc(size, align)`
  - `kfree(ptr)` (asserts on invalid/free-state violations)

## Diagnostics and invariants

- `POOS_ASSERT(condition)` panics with expression + `file:line`.
- Exception path decodes page faults (CR2 + error code).
- `mem_sanity_check()` verifies PMM allocation/free behavior.
- Boot logs include E820 summary, PMM stats, CR0/CR3 paging status.
- Heap + VMM smoke tests run during kernel init.

## Initialization order (v0.2)

`kernel_main()`:

1. Early VGA init
2. GDT/IDT/PIC init
3. BootInfo parse + kernel physical bounds publish
4. PMM init
5. VMM init
6. Heap init
7. Memory sanity + smoke tests
8. PIT init and interrupts enable
9. Memory summary print
10. Idle loop

## Build and run

```bash
make build
make run
```

Toolchain defaults to `i686-elf-*`; for local validation with host tools you can override:

```bash
make CROSS=i686-elf CC=gcc LD=ld OBJCOPY=objcopy build
```

## Forward path (v0.3+)

This architecture is now ready for:

- per-process page directories and user/supervisor separation
- copy-on-write and demand paging
- slab allocators over PMM frames
- scheduler + task address spaces
- SMP-safe locking around PMM/VMM/heap
