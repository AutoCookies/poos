[bits 16]
org 0x7C00

%define KERNEL_PHYS_BASE 0x00200000
%define REALMODE_STACK   0x7C00
%define PMODE_STACK      0x0009FC00
%define KERNEL_LBA_START 1
%define KERNEL_SECTORS   256
%define INITRD_LOAD_ADDR 0x00400000
%define INITRD_LBA_START 300
%define INITRD_SECTORS   1024
%define BOOTINFO_ADDR    0x9000

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, REALMODE_STACK
    mov [boot_drive], dl

    ; Load Kernel (128KB) to 0x1000:0000 (0x10000)
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov eax, KERNEL_LBA_START
    mov cx, KERNEL_SECTORS
    call bios_read_sectors

    ; Load Initrd (512KB) to 0x3000:0000 (0x30000)
    mov ax, 0x3000
    mov es, ax
    xor bx, bx
    mov eax, INITRD_LBA_START
    mov cx, INITRD_SECTORS
    call bios_read_sectors

    call enable_a20
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:protected_mode_entry

bios_read_sectors:
    pushad
.loop:
    push cx
    push eax
    ; LBA to CHS (63 SPT, 16 Heads)
    mov ecx, 1008 ; 63 * 16
    xor edx, edx
    div ecx ; EAX=C, EDX=LBA%(H*S)
    mov ch, al
    mov al, ah
    shl al, 6
    mov ah, al
    mov eax, edx
    mov ecx, 63
    xor edx, edx
    div ecx ; EAX=H, EDX=S-1
    mov dh, al
    mov cl, dl
    inc cl
    or cl, ah
    mov ax, 0x0201 ; Read 1 sector
    mov dl, [boot_drive]
    int 0x13
    jc .error
    pop eax
    inc eax
    pop cx
    add bx, 512
    jnz .no_inc_es
    mov ax, es
    add ax, 0x1000
    mov es, ax
.no_inc_es:
    loop .loop
    popad
    ret
.error:
    mov al, 'E'
    out 0xE9, al
    jmp $

enable_a20:
    in al, 0x92
    or al, 0x02
    out 0x92, al
    ret

[bits 32]
protected_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, PMODE_STACK

    ; Move Kernel to 2MB
    mov esi, 0x10000
    mov edi, KERNEL_PHYS_BASE
    mov ecx, (KERNEL_SECTORS * 512 / 4)
    rep movsd

    ; Move Initrd to 4MB
    mov esi, 0x30000
    mov edi, INITRD_LOAD_ADDR
    mov ecx, (INITRD_SECTORS * 512 / 4)
    rep movsd

    ; Update BOOTINFO
    mov di, BOOTINFO_ADDR
    mov dword [di + 0], 0x534F4F50
    mov dword [di + 4], 2
    mov dword [di + 8], 0
    mov dword [di + 12], 0
    mov dword [di + 16], KERNEL_PHYS_BASE
    mov dword [di + 20], 0
    mov dword [di + 24], INITRD_LOAD_ADDR
    mov dword [di + 28], (INITRD_SECTORS * 512)

    mov eax, BOOTINFO_ADDR
    jmp 0x08:KERNEL_PHYS_BASE

%include "boot/gdt.asm"
boot_drive: db 0
times 510 - ($ - $$) db 0
dw 0xAA55
