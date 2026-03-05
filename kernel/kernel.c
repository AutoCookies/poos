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

static void panic_boot(const char* msg) {
    vga_write("panic: ");
    vga_write(msg);
    vga_write("\n");
    for (;;) cpu_hlt();
}

static int require_path(const char* path) {
    struct file* f = 0;
    if (vfs_open(path, 0, &f) < 0) {
        vga_write("boot-check: missing ");
        vga_write(path);
        vga_write("\n");
        return -1;
    }
    file_put(f);
    vga_write("boot-check: found ");
    vga_write(path);
    vga_write("\n");
    return 0;
}

static int launch_init_chain(void) {
    const char* candidates[] = { "/sbin/init", "/bin/init", "/bin/sh" };
    for (u32 i = 0; i < (sizeof(candidates) / sizeof(candidates[0])); ++i) {
        vga_write("launching init candidate: path=");
        vga_write(candidates[i]);
        vga_write("\n");
        if (proc_spawn_path(candidates[i], 0) == 0) {
            vga_write("launch init success: path=");
            vga_write(candidates[i]);
            vga_write("\n");
            return 0;
        }
        vga_write("launch init failed: path=");
        vga_write(candidates[i]);
        vga_write("\n");
    }
    return -1;
}

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
    outb(0xE9, 'M');
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

    vga_write("mount: preparing root from initrd\n");
    vfs_init();
    bcache_init();
    initrd_set_region(bootinfo->initrd_phys_start, bootinfo->initrd_size);

    struct vnode* root = 0;
    if (tarfs_mount_root(initrd_data(), initrd_size(), &root) < 0 || vfs_mount("/", root) < 0) {
        panic_boot("failed to mount initrd root");
    }
    vga_write("mount: rootfs mounted at /\n");

    if (vfs_mount("/dev", devfs_root()) < 0) {
        panic_boot("failed to mount /dev");
    }
    vga_write("mount: devfs mounted at /dev\n");

    if (vfs_mount("/tmp", memfs_root()) < 0) {
        panic_boot("failed to mount /tmp");
    }
    vga_write("mount: memfs mounted at /tmp\n");

    if (require_path("/etc/passwd") < 0 || require_path("/bin/sh") < 0) {
        panic_boot("runtime sanity checks failed");
    }

    if (ata_pio_init() == 0) {
        struct blkdev* hd1 = blkdev_get("hd1");
        if (hd1 && part_scan_mbr(hd1) == 0) {
            struct blkdev* p1 = blkdev_get("hd1p1");
            struct vnode* droot = 0;
            if (p1 && fat_mount(p1, &droot) == 0 && vfs_mount("/home", droot) == 0) {
                vga_write("mounted /home from FAT disk\n");
            } else {
                vga_write("disk: FAT mount failed\n");
            }
        } else {
            vga_write("disk: no MBR partition on hd1\n");
        }
    } else {
        vga_write("disk: ata not found\n");
    }

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

    if (launch_init_chain() < 0) {
        panic_boot("no usable init binary (/sbin/init, /bin/init, /bin/sh)");
    }

    irq_enable();
    sched_start();

    for (;;) cpu_hlt();
}
