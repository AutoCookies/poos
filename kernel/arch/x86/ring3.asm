[bits 32]
global ring3_enter

; void ring3_enter(u32 eip, u32 useresp)
ring3_enter:
    cli
    mov eax, [esp + 4]
    mov edx, [esp + 8]

    mov bx, 0x23
    mov ds, bx
    mov es, bx
    mov fs, bx
    mov gs, bx

    push dword 0x23
    push edx
    push dword 0x202
    push dword 0x1B
    push eax
    iretd
