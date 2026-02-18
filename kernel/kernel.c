#include "types.h"
#include "bootinfo.h"
#include "mem/mem.h"
#include "mem/pmm.h"
#include "mem/paging.h"
#include "mem/vmm.h"
#include "mem/heap.h"
#include "arch/x86/cpu.h"
#include "arch/x86/irq.h"
#include "arch/x86/gdt.h"
#include "sched/sched.h"
#include "time/time.h"
#include "proc/proc.h"
#include "syscall/syscall.h"
#include "vfs/vfs.h"
#include "fs/initrd.h"
#include "fs/tarfs.h"
#include "fs/devfs.h"

void vga_init(void);
void vga_write(const char* s);
void vga_write_u32(u32 value);

extern u32 __kernel_phys_start;
extern u32 __kernel_phys_end;

static void ticker(void* arg) {
    (void)arg;
    for (;;) {
        kthread_sleep(time_ms_to_ticks(1000U));
        proc_reap_zombies();
    }
}

void kernel_main(struct BootInfo* bootinfo) {
    irq_disable();

    vga_init();
    vga_write("PoOS v0.5 booting...\n");

    gdt_init();
    irq_init();

    bootinfo->kernel_phys_start = (u32)&__kernel_phys_start;
    bootinfo->kernel_phys_end = (u32)&__kernel_phys_end;

    mem_init(bootinfo);
    mem_sanity_check();
    heap_smoke_test();

    time_init();
    sched_init();
    proc_init();
    syscall_init();
    pit_init();

    vfs_init();
    initrd_set_region(bootinfo->initrd_phys_start, bootinfo->initrd_size);

    struct vnode* root = 0;
    if (tarfs_mount_root(initrd_data(), initrd_size(), &root) < 0 || vfs_mount("/", root) < 0) {
        vga_write("panic: failed to mount initrd root\n");
        for (;;) cpu_hlt();
    }
    if (vfs_mount("/dev", devfs_root()) < 0) {
        vga_write("panic: failed to mount /dev\n");
        for (;;) cpu_hlt();
    }
    vga_write("mounted / from initrd (tarfs)\n");

    struct file* motd = 0;
    if (vfs_open("/etc/motd", 0, &motd) == 0) {
        char b[32];
        int n = motd->vnode->ops->read(motd->vnode, 0, b, sizeof(b) - 1);
        if (n > 0) { b[n] = '\0'; vga_write("motd: "); vga_write(b); vga_write("\n"); }
        file_put(motd);
    }

    kthread_create("ticker", ticker, 0, 0);

    vga_write("launching /sbin/init\n");
    if (proc_spawn_path("/sbin/init", 0) < 0) {
        vga_write("panic: /sbin/init missing\n");
        for (;;) cpu_hlt();
    }

    irq_enable();
    sched_start();

    for (;;) cpu_hlt();
}
