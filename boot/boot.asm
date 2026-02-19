[bits 16]
org 0x7C00

%define KERNEL_LOAD_ADDR 0x00100000
%define REALMODE_STACK   0x7C00
%define PMODE_STACK      0x0009FC00
%define KERNEL_LBA_START 1
%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS   256
%endif
%define INITRD_LOAD_ADDR 0x00180000
%define INITRD_LBA_START 300
%ifndef INITRD_SECTORS
%define INITRD_SECTORS   512
%endif
%ifndef INITRD_BYTES
%define INITRD_BYTES     (INITRD_SECTORS * 512)
%endif
%define BOOTINFO_ADDR    0x9000
%define E820_ENTRIES_MAX 128

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, REALMODE_STACK

    mov [boot_drive], dl

    call build_bootinfo
    call enable_a20

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp 0x08:protected_mode_entry

build_bootinfo:
    mov di, BOOTINFO_ADDR
    mov dword [di + 0], 0x534F4F50
    mov dword [di + 4], 2
    mov dword [di + 8], 0
    mov dword [di + 12], 0
    mov dword [di + 16], KERNEL_LOAD_ADDR
    mov dword [di + 20], 0
    mov dword [di + 24], INITRD_LOAD_ADDR
    mov dword [di + 28], INITRD_BYTES

    mov di, BOOTINFO_ADDR + 32
    xor ebx, ebx
    xor bp, bp
.e820_loop:
    mov eax, 0xE820
    mov edx, 0x534D4150
    mov ecx, 24
    int 0x15
    jc .done
    cmp eax, 0x534D4150
    jne .done

    inc bp
    add di, 24
    cmp bp, E820_ENTRIES_MAX
    jae .done
    test ebx, ebx
    jnz .e820_loop
.done:
    mov dword [BOOTINFO_ADDR + 12], ebp
    cmp bp, 0
    je .ret
    mov eax, [BOOTINFO_ADDR + 8]
    or eax, 1
    mov [BOOTINFO_ADDR + 8], eax
.ret:
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

    mov dl, [boot_drive]
    mov eax, KERNEL_LBA_START
    mov ecx, KERNEL_SECTORS
    mov edi, KERNEL_LOAD_ADDR
    call ata_lba_read

    mov dl, [boot_drive]
    mov eax, INITRD_LBA_START
    mov ecx, INITRD_SECTORS
    mov edi, INITRD_LOAD_ADDR
    call ata_lba_read

    mov eax, BOOTINFO_ADDR
    jmp 0x08:KERNEL_LOAD_ADDR

hang:
    cli
    hlt
    jmp hang

enable_a20:
    in al, 0x92
    test al, 0x02
    jnz .done
    or al, 0x02
    and al, 0xFE
    out 0x92, al
.done:
    ret

ata_lba_read:
    pushad
.next_sector:
    test ecx, ecx
    jz .done
    call ata_wait_not_busy

    mov dx, 0x1F2
    mov al, 1
    out dx, al

    mov ebx, eax
    mov dx, 0x1F3
    mov al, bl
    out dx, al

    mov dx, 0x1F4
    mov al, bh
    out dx, al

    shr ebx, 16
    mov dx, 0x1F5
    mov al, bl
    out dx, al

    mov dx, 0x1F6
    mov al, 0xE0
    and bh, 0x0F
    or al, bh
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
.wait1:
    in al, dx
    test al, 0x80
    jnz .wait1
    ret

ata_wait_drq:
    mov dx, 0x1F7
.wait2:
    in al, dx
    test al, 0x08
    jnz .ready
    test al, 0x01
    jnz hang
    jmp .wait2
.ready:
    ret

%include "boot/gdt.asm"

boot_drive: db 0

times 510 - ($ - $$) db 0
dw 0xAA55
