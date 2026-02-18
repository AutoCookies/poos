[bits 32]
extern main
global _start
_start:
    call main
    mov ebx, eax
    mov eax, 2
    int 0x80
.hang:
    jmp .hang
