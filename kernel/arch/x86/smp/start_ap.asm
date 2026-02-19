BITS 16
global start_ap_trampoline
start_ap_trampoline:
    cli
.hang:
    hlt
    jmp .hang
