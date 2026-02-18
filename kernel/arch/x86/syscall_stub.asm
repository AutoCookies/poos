[bits 32]
; int 0x80 uses generic ISR stubs in this revision.
section .text
global syscall_stub_placeholder
syscall_stub_placeholder:
    ret
