#include "irq.h"
#include "cpu.h"
#include "../../proc/task.h"
#include "../../proc/proc.h"

void panic(const char* msg);
void vga_write(const char* s);
void vga_write_u32(u32 v);
void vga_write_hex(u32 v);

void faults_handle_page_fault(struct trapframe* tf) {
    u32 fault_addr = cpu_read_cr2();
    bool user = (tf->cs & 0x3U) == 0x3U;

    vga_write("Page fault addr="); vga_write_hex(fault_addr);
    vga_write(" err="); vga_write_hex(tf->err_code);
    vga_write(" eip="); vga_write_hex(tf->eip);
    if (user) {
        struct task* t = task_current();
        vga_write(" pid="); vga_write_u32(t && t->owner ? t->owner->pid : 0U);
        vga_write(" [user]\n");
        proc_send_signal(t && t->owner ? t->owner->pid : 0U, SIGSEGV);
        proc_kill_current(-11);
        return;
    }
    vga_write(" [kernel]\n");
    panic("kernel page fault");
}
