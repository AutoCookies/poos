[bits 32]
section .text
global context_switch

; void context_switch(u32** prev_sp_out, u32* next_sp)
context_switch:
    mov eax, [esp + 4]
    mov edx, [esp + 8]

    pushfd
    push ebx
    push esi
    push edi
    push ebp

    mov [eax], esp
    mov esp, edx

    pop ebp
    pop edi
    pop esi
    pop ebx
    popfd
    ret
