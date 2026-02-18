#include "syscall.h"
#include "../arch/x86/idt.h"

void syscall_dispatch(struct trapframe* tf);

static void syscall_handler(struct trapframe* tf) {
    syscall_dispatch(tf);
}

void syscall_init(void) {
    idt_register_handler(0x80U, syscall_handler);
}
