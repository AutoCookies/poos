#include "types.h"
#include "bootinfo.h"
#include "mem/mem.h"
#include "mem/pmm.h"
#include "mem/paging.h"
#include "mem/vmm.h"
#include "mem/heap.h"
#include "arch/x86/cpu.h"
#include "arch/x86/irq.h"
#include "arch/x86/smp/smp.h"
#include "arch/x86/gdt.h"
#include "sched/sched.h"
#include "time/time.h"
#include "proc/proc.h"
#include "syscall/syscall.h"
#include "vfs/vfs.h"
#include "fs/initrd.h"
#include "fs/tarfs.h"
#include "fs/devfs.h"
#include "fs/memfs.h"
#include "tty/tty.h"
#include "mm/mm.h"
#include "mm/page.h"
#include "mm/budget.h"
#include "blk/blkdev.h"
#include "blk/part.h"
#include "bcache/bcache.h"
#include "fs/fat/fat.h"
#include "pci/pci.h"
#include "net/net.h"
#include "crypto/rng.h"

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

int ata_pio_init(void);
int rtl8139_init(void);

void kernel_main(struct BootInfo* bootinfo) {
    irq_disable();

    vga_init();
    vga_write("PoOS v1.1 booting...\n");

    gdt_init();
    irq_init();
    tty_init();
    kbd_init();

    bootinfo->kernel_phys_start = (u32)&__kernel_phys_start;
    bootinfo->kernel_phys_end = (u32)&__kernel_phys_end;

    mem_init(bootinfo);
    mem_sanity_check();
    mm_budget_init();
    heap_smoke_test();

    time_init();
    rng_init();
    rng_mix_entropy(bootinfo, sizeof(*bootinfo));
    sched_init();
    page_init();
    mm_init();
    proc_init();
    syscall_init();
    pit_init();
    smp_init();
    smp_boot_aps();

    vfs_init();
    bcache_init();
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
    if (vfs_mount("/tmp", memfs_root()) < 0) {
        vga_write("panic: failed to mount /tmp\n");
        for (;;) cpu_hlt();
    }
    if (ata_pio_init() == 0) {
        struct blkdev* hd0 = blkdev_get("hd0");
        if (hd0 && part_scan_mbr(hd0) == 0) {
            struct blkdev* p1 = blkdev_get("hd0p1");
            struct vnode* droot = 0;
            if (p1 && fat_mount(p1, &droot) == 0 && vfs_mount("/home", droot) == 0) {
                vga_write("mounted /home from FAT disk\n");
            } else {
                vga_write("disk: FAT mount failed\n");
            }
        } else {
            vga_write("disk: no MBR partition\n");
        }
    } else {
        vga_write("disk: ata not found\n");
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
    pci_init();
    net_init();
    if (rtl8139_init() == 0) {
        vga_write("net: rtl8139 ready\n");
    } else {
        vga_write("net: no rtl8139\n");
    }


    vga_write("launching /sbin/init\n");
    if (proc_spawn_path("/sbin/init", 0) < 0) {
        vga_write("panic: /sbin/init missing\n");
        for (;;) cpu_hlt();
    }

    irq_enable();
    sched_start();

    for (;;) cpu_hlt();
}
