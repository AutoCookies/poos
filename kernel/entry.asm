[bits 32]
%define KERNEL_VIRT_BASE 0xC0000000
%define PAGE_PRESENT 0x001
%define PAGE_RW      0x002

global _start
extern kernel_main

section .text
_start:
    cli

    mov [bootinfo_ptr], eax

    mov ecx, 1024
    xor eax, eax
    mov edi, boot_page_directory - KERNEL_VIRT_BASE
    rep stosd

    mov ecx, 1024
    mov edi, boot_low_page_table - KERNEL_VIRT_BASE
    xor ebx, ebx
.map_low:
    mov eax, ebx
    or eax, PAGE_PRESENT | PAGE_RW
    mov [edi], eax
    add ebx, 0x1000
    add edi, 4
    loop .map_low

    mov eax, boot_low_page_table - KERNEL_VIRT_BASE
    or eax, PAGE_PRESENT | PAGE_RW
    mov [boot_page_directory - KERNEL_VIRT_BASE + 0], eax
    mov [boot_page_directory - KERNEL_VIRT_BASE + 768*4], eax

    mov eax, boot_page_directory - KERNEL_VIRT_BASE
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    jmp 0x08:higher_half_entry

higher_half_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, stack_top

    push dword [bootinfo_ptr]
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang

section .bss
align 4096
boot_page_directory:
    resd 1024
boot_low_page_table:
    resd 1024

align 16
stack_bottom:
    resb 32768
stack_top:

bootinfo_ptr:
    resd 1

section .note.GNU-stack noalloc noexec nowrite progbits
