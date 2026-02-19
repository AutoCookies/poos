CROSS ?= i686-elf
AS := nasm

ifeq ($(shell command -v $(CROSS)-gcc >/dev/null 2>&1; echo $$?),0)
CC := $(CROSS)-gcc
LD := $(CROSS)-ld
OBJCOPY := $(CROSS)-objcopy
TOOLCHAIN_DESC := cross ($(CROSS))
else ifeq ($(shell command -v gcc >/dev/null 2>&1; echo $$?),0)
CC := gcc
LD := ld
OBJCOPY := objcopy
TOOLCHAIN_DESC := host (gcc/binutils fallback)
else
$(error No usable C compiler found. Install $(CROSS)-gcc or gcc)
endif

QEMU := qemu-system-i386
PYTHON ?= python3

CFLAGS := -std=c11 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -Wall -Wextra -Werror -m32 -O2
LDFLAGS := -T linker.ld -nostdlib -m elf_i386
U_CFLAGS := -std=c11 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -m32 -O2 -Wall -Wextra -Werror

PROFILE ?= edge80

# Host GCC is stricter about one-line formatting in legacy sources.
# Keep -Werror, but suppress this style-only warning for host fallback builds.
ifeq ($(TOOLCHAIN_DESC),host (gcc/binutils fallback))
CFLAGS += -Wno-misleading-indentation -Wno-missing-field-initializers
U_CFLAGS += -Wno-misleading-indentation -Wno-missing-field-initializers
endif
ifeq ($(PROFILE),edge80)
CFLAGS += -DCONFIG_EDGE_80MB=1 -DCONFIG_LEAN_SERVER=1
U_CFLAGS += -DCONFIG_EDGE_80MB=1 -DCONFIG_LEAN_SERVER=1
else ifeq ($(PROFILE),lean)
CFLAGS += -DCONFIG_LEAN_SERVER=1
U_CFLAGS += -DCONFIG_LEAN_SERVER=1
else
CFLAGS += -DCONFIG_FULL=1
U_CFLAGS += -DCONFIG_FULL=1
endif

BUILD_DIR := build
BOOT_BIN := $(BUILD_DIR)/boot.bin
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin
IMAGE := $(BUILD_DIR)/poos.img
ISO := $(BUILD_DIR)/poos.iso
ISO_ROOT := $(BUILD_DIR)/iso-root
DATA_IMAGE := $(BUILD_DIR)/poos_disk.img
INITRD_TAR := $(BUILD_DIR)/initrd.tar
ROOTFS_DIR := user/pack/rootfs
INITRD_LBA := 300

USER_APPS := init sh ls cat hello sleep fault cowtest mmaptest filemaptest mkdir rm mv cp sync ifconfig ping udpsend udprecv dnslookup httpget httpsget tlsprobe tcptest schedtest iotest nettest mmtest cpustat ps iostat locks login su id chmod chown umask passwd poosrun netnsctl memstat proxyd edge proxystat healthcheck
USER_ELFS := $(patsubst %,$(BUILD_DIR)/user/%.elf,$(USER_APPS))
USER_COMMON_OBJS := $(BUILD_DIR)/user/crt0.o $(BUILD_DIR)/user/libc_min/syscall.o $(BUILD_DIR)/user/libc_min/printf_min.o $(BUILD_DIR)/user/libc_min/string.o
PROXYD_SRCS := proxyd proxy_conn proxy_http1 proxy_tls proxy_cache_mem proxy_cache_disk proxy_eviction proxy_limits proxy_timeouts proxy_ratelimit proxy_health proxy_reload proxy_stats proxy_log
PROXYD_OBJS := $(patsubst %,$(BUILD_DIR)/user/apps/proxyd/%.o,$(PROXYD_SRCS))
EDGE_SRCS := init_edge svc_supervisor svc_config svc_health edge_bench edge_report edge_limits edge_time edge_health edge_tune
EDGE_OBJS := $(patsubst %,$(BUILD_DIR)/user/apps/edge/%.o,$(EDGE_SRCS))

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
	kernel/vfs/vfs_perm.c kernel/vfs/vnode.c kernel/vfs/vfs.c kernel/vfs/path.c kernel/vfs/file.c kernel/vfs/fdtable.c kernel/vfs/mount.c kernel/vfs/vfs_debug.c \
	kernel/fs/initrd.c kernel/fs/tarfs.c kernel/fs/devfs.c kernel/fs/memfs.c \
	kernel/blk/blkdev.c kernel/blk/bio.c kernel/blk/part.c kernel/blk/blk_debug.c \
	kernel/drivers/ata_pio.c kernel/drivers/virtio_blk.c kernel/arch/x86/smp/smp.c kernel/arch/x86/smp/apic.c kernel/arch/x86/smp/lapic.c kernel/arch/x86/smp/ioapic.c kernel/arch/x86/smp/ipi.c kernel/arch/x86/smp/cpu.c kernel/arch/x86/smp/per_cpu.c kernel/arch/x86/smp/gdt_percpu.c kernel/arch/x86/smp/tss_percpu.c kernel/arch/x86/smp/traps_percpu.c kernel/arch/x86/smp/mp_table.c kernel/arch/x86/smp/acpi_madt.c \
	kernel/sched/smp/sched_smp.c kernel/sched/smp/runqueue_percpu.c kernel/sched/smp/load_balance.c kernel/sched/smp/preempt.c \
	kernel/locks/spinlock.c kernel/locks/rwlock.c kernel/locks/mutex.c kernel/locks/lock_debug.c kernel/time/clocksource.c kernel/time/timerwheel.c kernel/time/time_smp.c kernel/irq/irq.c kernel/irq/irq_affinity.c kernel/irq/softirq.c \
	kernel/bcache/bcache.c kernel/bcache/lru.c kernel/bcache/writeback.c kernel/bcache/bcache_debug.c kernel/fs/fat/fat.c kernel/fs/fat/fat_dir.c kernel/fs/fat/fat_file.c kernel/fs/fat/fat_alloc.c kernel/fs/fat/fat_debug.c kernel/ipc/ringbuf.c kernel/ipc/pipe.c kernel/tty/tty.c kernel/tty/kbd.c kernel/tty/console.c \
	kernel/proc/signal.c kernel/proc/proc_table.c kernel/proc/fork.c kernel/proc/thread_user.c kernel/proc/mm_clone.c kernel/mm/addrspace.c kernel/mm/vma.c kernel/mm/page.c kernel/mm/cow.c kernel/mm/mmap.c kernel/mm/faults_vm.c kernel/mm/pagecache.c kernel/mm/anon.c kernel/mm/filemap.c kernel/mm/tlb.c kernel/mm/mm_debug.c kernel/mm/budget.c \
	kernel/pci/pci.c kernel/net/net.c kernel/net/net_timer.c kernel/net/net_stats.c kernel/net/netif.c kernel/net/pbuf.c kernel/net/checksum.c kernel/net/eth.c kernel/net/arp.c kernel/net/ipv4.c kernel/net/icmp.c kernel/net/udp.c kernel/net/dhcp.c kernel/net/dns.c kernel/net/route.c kernel/net/sock.c kernel/net/sock_api.c kernel/net/net_debug.c kernel/net/tcp/tcp.c kernel/net/tcp/tcp_state.c kernel/net/tcp/tcp_input.c kernel/net/tcp/tcp_output.c kernel/net/tcp/tcp_timer.c kernel/net/tcp/tcp_retransmit.c kernel/net/tcp/tcp_window.c kernel/net/tcp/tcp_conn.c kernel/net/tcp/tcp_sock.c kernel/net/tcp/tcp_debug.c kernel/drivers/rtl8139.c kernel/drivers/virtio_net.c kernel/dev/devnet.c kernel/crypto/memwipe.c kernel/crypto/constant_time.c kernel/crypto/rng.c kernel/crypto/sha256.c kernel/crypto/hmac.c kernel/crypto/hkdf.c
KERNEL_ASM_SRCS := kernel/entry.asm kernel/arch/x86/isr_stubs.asm kernel/arch/x86/ring3.asm kernel/arch/x86/syscall_stub.asm kernel/sched/context_switch.asm kernel/arch/x86/smp/start_ap.asm
KERNEL_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_C_SRCS)) $(patsubst %.asm,$(BUILD_DIR)/%.o,$(KERNEL_ASM_SRCS))

.PHONY: all clean kernel user rootfs initrd iso verify-iso run run-headless test-full-build toolchain-check
all: iso

toolchain-check:
	@echo "[toolchain] $(TOOLCHAIN_DESC)"
	@echo "[toolchain] CC=$(CC)"
	@echo "[toolchain] LD=$(LD)"
	@echo "[toolchain] OBJCOPY=$(OBJCOPY)"

clean:
	rm -rf $(BUILD_DIR)

kernel: toolchain-check $(KERNEL_BIN)
	@test -f $(KERNEL_BIN) || (echo "error: missing $(KERNEL_BIN)" && exit 1)
	@echo "[ok] kernel built: $(KERNEL_BIN)"

user: $(USER_ELFS)
	@for app in $(USER_APPS); do test -f "$(BUILD_DIR)/user/$$app.elf" || (echo "error: missing user binary $$app" && exit 1); done
	@echo "[ok] userland built ($(words $(USER_APPS)) binaries)"

rootfs: user
	@echo "[stage] building rootfs"
	@mkdir -p $(ROOTFS_DIR)/bin $(ROOTFS_DIR)/sbin $(ROOTFS_DIR)/etc
	@rm -f $(ROOTFS_DIR)/bin/* $(ROOTFS_DIR)/sbin/*
	@cp $(BUILD_DIR)/user/init.elf $(ROOTFS_DIR)/sbin/init
	@for app in $(filter-out init,$(USER_APPS)); do cp "$(BUILD_DIR)/user/$$app.elf" "$(ROOTFS_DIR)/bin/$$app"; done
	@test -f $(ROOTFS_DIR)/etc/passwd || (echo "error: missing $(ROOTFS_DIR)/etc/passwd" && exit 1)
	@test -f $(ROOTFS_DIR)/etc/shadow || (echo "error: missing $(ROOTFS_DIR)/etc/shadow" && exit 1)
	@test -f $(ROOTFS_DIR)/etc/group || (echo "error: missing $(ROOTFS_DIR)/etc/group" && exit 1)
	@test -f $(ROOTFS_DIR)/sbin/init || (echo "error: missing $(ROOTFS_DIR)/sbin/init" && exit 1)
	@test -f $(ROOTFS_DIR)/bin/sh || (echo "error: missing $(ROOTFS_DIR)/bin/sh" && exit 1)
	@echo "[ok] rootfs staged in $(ROOTFS_DIR)"

initrd: rootfs user/pack/mkinitrd.sh
	@echo "[stage] packing initrd"
	@mkdir -p $(BUILD_DIR)
	@user/pack/mkinitrd.sh $(ROOTFS_DIR) $(INITRD_TAR)
	@test -f $(INITRD_TAR) || (echo "error: missing $(INITRD_TAR)" && exit 1)
	@tar -tf $(INITRD_TAR) | rg -q '^\./sbin/init$$' || (echo "error: initrd missing /sbin/init" && exit 1)
	@tar -tf $(INITRD_TAR) | rg -q '^\./bin/sh$$' || (echo "error: initrd missing /bin/sh" && exit 1)
	@echo "[ok] initrd created: $(INITRD_TAR)"

iso: kernel initrd $(BOOT_BIN) $(DATA_IMAGE)
	@echo "[stage] assembling boot image + iso artifact"
	@dd if=/dev/zero of=$(IMAGE) bs=512 count=4096 status=none
	@dd if=$(BOOT_BIN) of=$(IMAGE) conv=notrunc status=none
	@dd if=$(KERNEL_BIN) of=$(IMAGE) bs=512 seek=1 conv=notrunc status=none
	@dd if=$(INITRD_TAR) of=$(IMAGE) bs=512 seek=$(INITRD_LBA) conv=notrunc status=none
	@mkdir -p $(ISO_ROOT)/boot/grub
	@cp $(KERNEL_BIN) $(ISO_ROOT)/boot/kernel.bin
	@cp $(INITRD_TAR) $(ISO_ROOT)/boot/initrd.tar
	@cp boot/grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg
	@cp $(IMAGE) $(ISO)
	@$(MAKE) verify-iso
	@echo "[ok] iso artifact ready: $(ISO)"

verify-iso:
	@tools/verify_iso.sh $(ISO) $(ISO_ROOT) $(INITRD_TAR) $(KERNEL_BIN)

run: iso
	$(QEMU) -drive format=raw,file=$(ISO),if=ide,index=0 -drive format=raw,file=$(DATA_IMAGE),if=ide,index=1 -netdev user,id=n1,hostfwd=udp::5555-:5555 -device rtl8139,netdev=n1

run-headless: iso
	$(QEMU) -m 80M -smp 1 -no-reboot -no-shutdown -display none -serial none -debugcon stdio -global isa-debugcon.iobase=0xe9 -drive format=raw,file=$(ISO),if=ide,index=0 -drive format=raw,file=$(DATA_IMAGE),if=ide,index=1 -netdev user,id=n1,hostfwd=udp::5555-:5555 -device rtl8139,netdev=n1

test-full-build:
	@tools/test_full_build.sh

$(BOOT_BIN): boot/boot.asm boot/gdt.asm
	mkdir -p $(BUILD_DIR)
	$(AS) -f bin -o $@ boot/boot.asm

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(KERNEL_ELF): $(KERNEL_OBJS) linker.ld
	mkdir -p $(BUILD_DIR)
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

$(BUILD_DIR)/user/apps/proxyd/%.o: user/apps/proxyd/%.c
	mkdir -p $(dir $@)
	$(CC) $(U_CFLAGS) -c $< -o $@

$(BUILD_DIR)/user/apps/edge/%.o: user/apps/edge/%.c
	mkdir -p $(dir $@)
	$(CC) $(U_CFLAGS) -c $< -o $@

$(BUILD_DIR)/user/%.elf: $(USER_COMMON_OBJS) $(BUILD_DIR)/user/apps/%.o user/user.ld
	$(LD) -T user/user.ld -nostdlib -m elf_i386 -o $@ $(USER_COMMON_OBJS) $(BUILD_DIR)/user/apps/$*.o

$(BUILD_DIR)/user/proxyd.elf: $(USER_COMMON_OBJS) $(PROXYD_OBJS) user/user.ld
	$(LD) -T user/user.ld -nostdlib -m elf_i386 -o $@ $(USER_COMMON_OBJS) $(PROXYD_OBJS)

$(BUILD_DIR)/user/edge.elf: $(USER_COMMON_OBJS) $(EDGE_OBJS) user/user.ld
	$(LD) -T user/user.ld -nostdlib -m elf_i386 -o $@ $(USER_COMMON_OBJS) $(EDGE_OBJS)

$(DATA_IMAGE): tools/mkfatdisk.py
	mkdir -p $(BUILD_DIR)
	$(PYTHON) tools/mkfatdisk.py $(DATA_IMAGE)
