[bits 32]
%define KERNEL_VIRT_BASE 0xC0000000
%define PAGE_PRESENT 0x001
%define PAGE_RW      0x002

global _start
extern kernel_main

section .text.entry
_start:
    cli
    mov [bootinfo_ptr - KERNEL_VIRT_BASE], eax

    ; Clear page directory
    mov ecx, 1024
    xor eax, eax
    mov edi, boot_page_directory - KERNEL_VIRT_BASE
    rep stosd

    ; Map first 16MB (4 tables)
    mov edx, 0 ; Physical address start
    mov edi, boot_tables - KERNEL_VIRT_BASE
    mov ecx, 4 ; 4 Tables
.map_table_loop:
    push ecx
    mov ecx, 1024
.map_page_loop:
    mov eax, edx
    or eax, PAGE_PRESENT | PAGE_RW
    mov [edi], eax
    add edx, 0x1000
    add edi, 4
    loop .map_page_loop
    pop ecx
    loop .map_table_loop

    ; Point PD to these tables
    mov ecx, 4
    mov edi, boot_page_directory - KERNEL_VIRT_BASE
    mov eax, boot_tables - KERNEL_VIRT_BASE
    or eax, PAGE_PRESENT | PAGE_RW
.pd_fill:
    mov [edi + 0], eax       ; Identity
    mov [edi + 768*4], eax   ; Higher Half
    add edi, 4
    add eax, 4096
    loop .pd_fill

    ; Enable paging
    mov eax, boot_page_directory - KERNEL_VIRT_BASE
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    lea eax, [higher_half_entry]
    jmp eax

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

align 4096
boot_page_directory:
    resd 1024
boot_tables:
    resd 4096 ; 4 tables = 16KB

align 16
stack_bottom:
    times 16384 db 0
stack_top:

bootinfo_ptr:
    dd 0

section .note.GNU-stack noalloc noexec nowrite progbits
