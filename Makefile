CROSS ?= i686-elf
CC := $(CROSS)-gcc
LD := $(CROSS)-ld
AS := nasm
OBJCOPY := $(CROSS)-objcopy
QEMU := qemu-system-i386

CFLAGS := -std=c11 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -Wall -Wextra -Werror -m32 -O2
LDFLAGS := -T linker.ld -nostdlib -m elf_i386
U_CFLAGS := -std=c11 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -m32 -O2 -Wall -Wextra -Werror

BUILD_DIR := build
BOOT_BIN := $(BUILD_DIR)/boot.bin
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin
IMAGE := $(BUILD_DIR)/poos.img
DATA_IMAGE := $(BUILD_DIR)/poos_disk.img
INITRD_TAR := $(BUILD_DIR)/initrd.tar

INITRD_LBA := 300

USER_APPS := init sh ls cat hello sleep fault cowtest mmaptest filemaptest mkdir rm mv cp sync ifconfig ping udpsend udprecv dnslookup httpget httpsget tlsprobe tcptest schedtest iotest nettest mmtest cpustat ps iostat locks login su id chmod chown umask passwd poosrun
USER_ELFS := $(patsubst %,$(BUILD_DIR)/user/%.elf,$(USER_APPS))
USER_COMMON_OBJS := $(BUILD_DIR)/user/crt0.o $(BUILD_DIR)/user/libc_min/syscall.o $(BUILD_DIR)/user/libc_min/printf_min.o $(BUILD_DIR)/user/libc_min/string.o

KERNEL_C_SRCS := \
	kernel/kernel.c kernel/gdt.c kernel/arch/x86/gdt.c kernel/arch/x86/tss.c kernel/arch/x86/idt.c \
	kernel/arch/x86/irq.c kernel/arch/x86/faults.c kernel/arch/x86/pic.c kernel/arch/x86/pit.c \
	kernel/time/time.c kernel/sched/sched.c kernel/sched/thread.c kernel/sched/runqueue.c kernel/sched/sleepq.c \
	kernel/vga.c kernel/panic.c kernel/mem/mem.c kernel/mem/e820.c kernel/mem/pmm.c kernel/mem/paging.c kernel/mem/vmm.c \
	kernel/mem/heap.c kernel/mem/mem_debug.c kernel/proc/proc.c kernel/proc/task.c kernel/proc/pid.c kernel/proc/elf32.c \
	kernel/proc/usercopy.c kernel/proc/ustack.c kernel/proc/exec.c kernel/proc/wait.c kernel/proc/reaper.c \
	kernel/syscall/syscall.c kernel/syscall/sys_dispatch.c kernel/syscall/sys_impl.c \
	kernel/sec/cred.c kernel/sec/auth.c kernel/sec/caps.c kernel/sec/audit.c kernel/sec/sec_debug.c \
	kernel/proc/proc_cred.c kernel/proc/exec_secure.c kernel/proc/ns.c kernel/proc/session.c \
	kernel/proc/clone.c kernel/proc/proc_ns.c kernel/proc/proc_cgroup.c \
	kernel/ns/ns.c kernel/ns/mntns.c kernel/ns/pidns.c kernel/ns/netns.c kernel/ns/utsns.c kernel/ns/userns.c kernel/ns/ns_proxy.c \
	kernel/cgroup/cgroup.c kernel/cgroup/cg_cpu.c kernel/cgroup/cg_mem.c kernel/cgroup/cg_pids.c kernel/cgroup/cg_debug.c \
	kernel/seccomp/seccomp.c kernel/seccomp/seccomp_rules.c kernel/seccomp/seccomp_debug.c \
	kernel/vfs/vfs_perm.c \
	kernel/vfs/vnode.c kernel/vfs/vfs.c kernel/vfs/path.c kernel/vfs/file.c kernel/vfs/fdtable.c kernel/vfs/mount.c kernel/vfs/vfs_debug.c \
	kernel/fs/initrd.c kernel/fs/tarfs.c kernel/fs/devfs.c kernel/fs/memfs.c \
	kernel/blk/blkdev.c kernel/blk/bio.c kernel/blk/part.c kernel/blk/blk_debug.c \
	kernel/drivers/ata_pio.c kernel/drivers/virtio_blk.c \
	kernel/arch/x86/smp/smp.c kernel/arch/x86/smp/apic.c kernel/arch/x86/smp/lapic.c kernel/arch/x86/smp/ioapic.c kernel/arch/x86/smp/ipi.c kernel/arch/x86/smp/cpu.c kernel/arch/x86/smp/per_cpu.c kernel/arch/x86/smp/gdt_percpu.c kernel/arch/x86/smp/tss_percpu.c kernel/arch/x86/smp/traps_percpu.c kernel/arch/x86/smp/mp_table.c kernel/arch/x86/smp/acpi_madt.c \
	kernel/sched/smp/sched_smp.c kernel/sched/smp/runqueue_percpu.c kernel/sched/smp/load_balance.c kernel/sched/smp/preempt.c \
	kernel/locks/spinlock.c kernel/locks/rwlock.c kernel/locks/mutex.c kernel/locks/lock_debug.c \
	kernel/time/clocksource.c kernel/time/timerwheel.c kernel/time/time_smp.c kernel/irq/irq.c kernel/irq/irq_affinity.c kernel/irq/softirq.c \
	kernel/bcache/bcache.c kernel/bcache/lru.c kernel/bcache/writeback.c kernel/bcache/bcache_debug.c \
	kernel/fs/fat/fat.c kernel/fs/fat/fat_dir.c kernel/fs/fat/fat_file.c kernel/fs/fat/fat_alloc.c kernel/fs/fat/fat_debug.c \
	kernel/ipc/ringbuf.c kernel/ipc/pipe.c kernel/tty/tty.c kernel/tty/kbd.c kernel/tty/console.c \
	kernel/proc/signal.c kernel/proc/proc_table.c kernel/proc/fork.c kernel/proc/thread_user.c kernel/proc/mm_clone.c \
	kernel/mm/addrspace.c kernel/mm/vma.c kernel/mm/page.c kernel/mm/cow.c kernel/mm/mmap.c kernel/mm/faults_vm.c kernel/mm/pagecache.c kernel/mm/anon.c kernel/mm/filemap.c kernel/mm/tlb.c kernel/mm/mm_debug.c \
	kernel/pci/pci.c kernel/net/net.c kernel/net/net_timer.c kernel/net/net_stats.c kernel/net/netif.c kernel/net/pbuf.c kernel/net/checksum.c kernel/net/eth.c kernel/net/arp.c kernel/net/ipv4.c kernel/net/icmp.c kernel/net/udp.c kernel/net/dhcp.c kernel/net/dns.c kernel/net/route.c kernel/net/sock.c kernel/net/sock_api.c kernel/net/net_debug.c kernel/net/tcp/tcp.c kernel/net/tcp/tcp_state.c kernel/net/tcp/tcp_input.c kernel/net/tcp/tcp_output.c kernel/net/tcp/tcp_timer.c kernel/net/tcp/tcp_retransmit.c kernel/net/tcp/tcp_window.c kernel/net/tcp/tcp_conn.c kernel/net/tcp/tcp_sock.c kernel/net/tcp/tcp_debug.c kernel/drivers/rtl8139.c kernel/drivers/virtio_net.c kernel/dev/devnet.c \
	kernel/crypto/memwipe.c kernel/crypto/constant_time.c kernel/crypto/rng.c kernel/crypto/sha256.c kernel/crypto/hmac.c kernel/crypto/hkdf.c

KERNEL_ASM_SRCS := kernel/entry.asm kernel/arch/x86/isr_stubs.asm kernel/arch/x86/ring3.asm kernel/arch/x86/syscall_stub.asm kernel/sched/context_switch.asm kernel/arch/x86/smp/start_ap.asm

KERNEL_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_C_SRCS)) $(patsubst %.asm,$(BUILD_DIR)/%.o,$(KERNEL_ASM_SRCS))

.PHONY: build run clean
build: $(IMAGE) $(DATA_IMAGE)

$(IMAGE): $(BOOT_BIN) $(KERNEL_BIN) $(INITRD_TAR)
	mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$(IMAGE) bs=512 count=4096 status=none
	dd if=$(BOOT_BIN) of=$(IMAGE) conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$(IMAGE) bs=512 seek=1 conv=notrunc status=none
	dd if=$(INITRD_TAR) of=$(IMAGE) bs=512 seek=$(INITRD_LBA) conv=notrunc status=none

$(INITRD_TAR): $(USER_ELFS) user/pack/mkinitrd.sh
	mkdir -p user/pack/rootfs/bin user/pack/rootfs/sbin
	cp $(BUILD_DIR)/user/init.elf user/pack/rootfs/sbin/init
	cp $(BUILD_DIR)/user/sh.elf user/pack/rootfs/bin/sh
	cp $(BUILD_DIR)/user/ls.elf user/pack/rootfs/bin/ls
	cp $(BUILD_DIR)/user/cat.elf user/pack/rootfs/bin/cat
	cp $(BUILD_DIR)/user/hello.elf user/pack/rootfs/bin/hello
	cp $(BUILD_DIR)/user/sleep.elf user/pack/rootfs/bin/sleep
	cp $(BUILD_DIR)/user/fault.elf user/pack/rootfs/bin/fault
	cp $(BUILD_DIR)/user/cowtest.elf user/pack/rootfs/bin/cowtest
	cp $(BUILD_DIR)/user/mmaptest.elf user/pack/rootfs/bin/mmaptest
	cp $(BUILD_DIR)/user/filemaptest.elf user/pack/rootfs/bin/filemaptest
	cp $(BUILD_DIR)/user/mkdir.elf user/pack/rootfs/bin/mkdir
	cp $(BUILD_DIR)/user/rm.elf user/pack/rootfs/bin/rm
	cp $(BUILD_DIR)/user/mv.elf user/pack/rootfs/bin/mv
	cp $(BUILD_DIR)/user/cp.elf user/pack/rootfs/bin/cp
	cp $(BUILD_DIR)/user/sync.elf user/pack/rootfs/bin/sync
	cp $(BUILD_DIR)/user/ifconfig.elf user/pack/rootfs/bin/ifconfig
	cp $(BUILD_DIR)/user/ping.elf user/pack/rootfs/bin/ping
	cp $(BUILD_DIR)/user/udpsend.elf user/pack/rootfs/bin/udpsend
	cp $(BUILD_DIR)/user/udprecv.elf user/pack/rootfs/bin/udprecv
	cp $(BUILD_DIR)/user/dnslookup.elf user/pack/rootfs/bin/dnslookup
	cp $(BUILD_DIR)/user/httpget.elf user/pack/rootfs/bin/httpget
	cp $(BUILD_DIR)/user/httpsget.elf user/pack/rootfs/bin/httpsget
	cp $(BUILD_DIR)/user/tlsprobe.elf user/pack/rootfs/bin/tlsprobe
	cp $(BUILD_DIR)/user/tcptest.elf user/pack/rootfs/bin/tcptest
	cp $(BUILD_DIR)/user/schedtest.elf user/pack/rootfs/bin/schedtest
	cp $(BUILD_DIR)/user/iotest.elf user/pack/rootfs/bin/iotest
	cp $(BUILD_DIR)/user/nettest.elf user/pack/rootfs/bin/nettest
	cp $(BUILD_DIR)/user/mmtest.elf user/pack/rootfs/bin/mmtest
	cp $(BUILD_DIR)/user/cpustat.elf user/pack/rootfs/bin/cpustat
	cp $(BUILD_DIR)/user/ps.elf user/pack/rootfs/bin/ps
	cp $(BUILD_DIR)/user/iostat.elf user/pack/rootfs/bin/iostat
	cp $(BUILD_DIR)/user/locks.elf user/pack/rootfs/bin/locks
	cp $(BUILD_DIR)/user/login.elf user/pack/rootfs/bin/login
	cp $(BUILD_DIR)/user/su.elf user/pack/rootfs/bin/su
	cp $(BUILD_DIR)/user/id.elf user/pack/rootfs/bin/id
	cp $(BUILD_DIR)/user/chmod.elf user/pack/rootfs/bin/chmod
	cp $(BUILD_DIR)/user/chown.elf user/pack/rootfs/bin/chown
	cp $(BUILD_DIR)/user/umask.elf user/pack/rootfs/bin/umask
	cp $(BUILD_DIR)/user/passwd.elf user/pack/rootfs/bin/passwd
	cp $(BUILD_DIR)/user/poosrun.elf user/pack/rootfs/bin/poosrun
	user/pack/mkinitrd.sh user/pack/rootfs $(INITRD_TAR)

$(BOOT_BIN): boot/boot.asm boot/gdt.asm
	mkdir -p $(BUILD_DIR)
	$(AS) -f bin -o $@ boot/boot.asm

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(KERNEL_ELF): $(KERNEL_OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: kernel/%.asm
	mkdir -p $(dir $@)
	$(AS) -f elf32 -o $@ $<

$(BUILD_DIR)/user/crt0.o: user/crt0.asm
	mkdir -p $(dir $@)
	$(AS) -f elf32 -o $@ $<

$(BUILD_DIR)/user/libc_min/syscall.o: user/libc_min/syscall.asm
	mkdir -p $(dir $@)
	$(AS) -f elf32 -o $@ $<

$(BUILD_DIR)/user/libc_min/%.o: user/libc_min/%.c
	mkdir -p $(dir $@)
	$(CC) $(U_CFLAGS) -c $< -o $@

$(BUILD_DIR)/user/apps/%.o: user/apps/%.c
	mkdir -p $(dir $@)
	$(CC) $(U_CFLAGS) -c $< -o $@

$(BUILD_DIR)/user/%.elf: $(USER_COMMON_OBJS) $(BUILD_DIR)/user/apps/%.o user/user.ld
	$(LD) -T user/user.ld -nostdlib -m elf_i386 -o $@ $(USER_COMMON_OBJS) $(BUILD_DIR)/user/apps/$*.o

run: $(IMAGE) $(DATA_IMAGE)
	$(QEMU) -drive format=raw,file=$(IMAGE),if=ide,index=0 -drive format=raw,file=$(DATA_IMAGE),if=ide,index=1 -netdev user,id=n1,hostfwd=udp::5555-:5555 -device rtl8139,netdev=n1

clean:
	rm -rf $(BUILD_DIR)

$(DATA_IMAGE): tools/mkfatdisk.py
	mkdir -p $(BUILD_DIR)
	python3 tools/mkfatdisk.py $(DATA_IMAGE)
