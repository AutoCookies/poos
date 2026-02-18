# PoOS v0.4

PoOS v0.4 adds the first real user-mode substrate on 32-bit x86 protected mode.

## Memory split
- User virtual address space: `< 0xC0000000`
- Kernel higher-half mapping: `>= 0xC0000000`
- User ELF load base: `0x08048000`
- User stack top: `0xBFFFE000`

## GDT/selectors
- `0x08`: kernel code
- `0x10`: kernel data
- `0x1B`: user code (RPL=3)
- `0x23`: user data (RPL=3)
- `0x28`: TSS

A hardware TSS is configured with `ss0`/`esp0` and updated on task switch.

## Syscall ABI (`int 0x80`)
- `eax`: syscall number
- `ebx`, `ecx`, `edx`, `esi`, `edi`: args
- return in `eax`
- syscalls:
  - `1 write(ptr,len)`
  - `2 exit(code)`
  - `3 yield()`
  - `4 sleep(ms)`
  - `5 getpid()`

IDT vector `0x80` is installed as DPL=3 gate.

## Trapframe
Kernel trapframe includes GPRs, vector/error code, and IRET frame (`eip/cs/eflags/useresp/ss`) for user transitions.

## ELF32 loading
Minimal static ELF32 loader supports `PT_LOAD` segments only.
- validates ELF magic + x86 machine
- maps user pages with `PAGE_USER`
- copies `filesz`
- zeros remaining segment memory (`bss`)
- sets initial user `EIP`

## User image packing
Makefile appends user ELF apps into fixed disk LBAs:
- hello: LBA 150
- fault: LBA 160

Bootloader reads first 256 sectors; kernel loads apps from known physical addresses in that window.

## Adding new user apps
1. Add `user/apps/<name>.c`
2. Add target in `Makefile` similar to hello/fault
3. Pick fixed LBA and kernel load physical address
4. Call `proc_spawn_user_image()` in `kernel_main`
