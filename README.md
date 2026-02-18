# PoOS v0.1

PoOS v0.1 is a minimal but production-minded educational 32-bit x86 kernel that boots directly via BIOS, transitions to protected mode, and initializes the core interrupt/timer/display subsystems needed for continued OS development.

## Architecture overview

PoOS is split into a strict two-stage architecture:

1. **Boot stage (`boot/`)**
   - BIOS loads the boot sector at `0x7C00`.
   - Boot code enables A20, loads a flat GDT, switches to 32-bit protected mode, then reads kernel sectors from disk using ATA PIO into physical `0x00100000`.
   - Control transfers to kernel entry at 1 MiB.

2. **Kernel stage (`kernel/`)**
   - Entry assembly establishes segment registers and a dedicated kernel stack.
   - `kernel_main()` initializes GDT, IDT, PIC remap, PIT timer, VGA text output, and global interrupts.
   - IRQ0 (timer) fires at 100 Hz and prints a periodic heartbeat.

## Memory layout

PoOS currently assumes this baseline memory map:

- `0x00000000 - 0x0009FFFF`: usable conventional RAM
- `0x000A0000 - 0x000FFFFF`: reserved (video BIOS / devices / ROM)
- `0x00100000+`: kernel image load + execution base

Additional fixed regions:

- Boot sector executes at `0x00007C00`.
- Boot protected-mode stack uses memory below 1 MiB (`0x0009FC00`).
- Kernel stack is defined in `.bss` and linked at/above 1 MiB.
- VGA text memory is mapped at `0x000B8000`.

## Boot flow

1. BIOS loads MBR sector and jumps to boot code.
2. Boot code initializes 16-bit environment and stack.
3. A20 is enabled through port `0x92`.
4. Boot-time GDT is loaded and protected mode is enabled (`CR0.PE=1`).
5. 32-bit boot code reads kernel sectors from LBA 1 into `0x00100000`.
6. Far jump to kernel entry (`_start`).
7. Kernel brings up descriptor tables, interrupts, timer, and console.

## Kernel initialization sequence

`kernel_main()` performs:

1. VGA console clear and early boot message.
2. Runtime GDT initialization and segment reload.
3. IDT installation with 256 ISR stubs.
4. PIC remap to vectors `0x20-0x2F` and IRQ mask clear.
5. PIT programming to 100 Hz (IRQ0 handler registered).
6. `sti` (interrupts enabled).
7. Idle loop with `hlt`.

## Cross-compiler setup

PoOS expects an `i686-elf` cross toolchain in `PATH`:

- `i686-elf-gcc`
- `i686-elf-ld`
- `i686-elf-objcopy`
- `nasm`
- `qemu-system-i386`

Typical GNU cross target tuple is `i686-elf`.

If your prefix differs, override at build time:

```bash
make CROSS=<your-prefix>
```

## Build and run

```bash
make build
make run
```

The build creates:

- `build/boot.bin` (exactly 512-byte boot sector with `0xAA55` signature)
- `build/kernel.elf` (linked kernel ELF)
- `build/kernel.bin` (flat kernel image)
- `build/poos.img` (bootable raw disk image for QEMU)

## Project layout

```
boot/
  boot.asm
  gdt.asm
kernel/
  entry.asm
  kernel.c
  gdt.c
  idt.c
  isr.asm
  timer.c
  vga.c
  panic.c
  types.h
linker.ld
Makefile
README.md
```

## Current status

PoOS v0.1 provides a clean base for future additions: paging, physical/virtual memory allocators, scheduler, syscall ABI, user mode transitions, ELF loading, filesystems, and SMP.
