[bits 16]
org 0x7C00

%define KERNEL_LOAD_ADDR 0x00100000
%define BOOT_DRIVE_ADDR  0x7C00
%define REALMODE_STACK   0x7C00
%define PMODE_STACK      0x0009FC00
%define KERNEL_LBA_START 1
%define KERNEL_SECTORS   128

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, REALMODE_STACK

    mov [boot_drive], dl

    call enable_a20

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp 0x08:protected_mode_entry

[bits 32]
protected_mode_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, PMODE_STACK

    mov dl, [boot_drive]
    mov eax, KERNEL_LBA_START
    mov ecx, KERNEL_SECTORS
    mov edi, KERNEL_LOAD_ADDR
    call ata_lba_read

    jmp 0x08:KERNEL_LOAD_ADDR

hang:
    cli
    hlt
    jmp hang

; Fast A20 gate via port 0x92
enable_a20:
    in al, 0x92
    test al, 0x02
    jnz .done
    or al, 0x02
    and al, 0xFE
    out 0x92, al
.done:
    ret

; Read ECX sectors from EAX LBA to EDI using ATA PIO, primary master.
ata_lba_read:
    pushad
.next_sector:
    cmp ecx, 0
    je .done

    call ata_wait_not_busy

    mov dx, 0x1F2
    mov al, 1
    out dx, al

    mov edx, eax
    mov dx, 0x1F3
    mov al, dl
    out dx, al

    mov dx, 0x1F4
    mov al, dh
    out dx, al

    shr edx, 16
    mov dx, 0x1F5
    mov al, dl
    out dx, al

    mov dx, 0x1F6
    mov al, 0xE0
    or al, dh
    out dx, al

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    call ata_wait_drq

    mov dx, 0x1F0
    mov ebx, 256
.read_word:
    in ax, dx
    mov [edi], ax
    add edi, 2
    dec ebx
    jnz .read_word

    inc eax
    dec ecx
    jmp .next_sector

.done:
    popad
    ret

ata_wait_not_busy:
    mov dx, 0x1F7
.wait:
    in al, dx
    test al, 0x80
    jnz .wait
    ret

ata_wait_drq:
    mov dx, 0x1F7
.wait:
    in al, dx
    test al, 0x08
    jnz .ready
    test al, 0x01
    jnz hang
    jmp .wait
.ready:
    ret

%include "boot/gdt.asm"

boot_drive: db 0

times 510 - ($ - $$) db 0
dw 0xAA55
