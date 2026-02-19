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

USER_APPS := init sh ls cat hello sleep fault cowtest mmaptest filemaptest mkdir rm mv cp sync
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
	kernel/vfs/vnode.c kernel/vfs/vfs.c kernel/vfs/path.c kernel/vfs/file.c kernel/vfs/fdtable.c kernel/vfs/mount.c kernel/vfs/vfs_debug.c \
	kernel/fs/initrd.c kernel/fs/tarfs.c kernel/fs/devfs.c kernel/fs/memfs.c \
	kernel/blk/blkdev.c kernel/blk/bio.c kernel/blk/part.c kernel/blk/blk_debug.c \
	kernel/drivers/ata_pio.c kernel/drivers/virtio_blk.c \
	kernel/bcache/bcache.c kernel/bcache/lru.c kernel/bcache/writeback.c kernel/bcache/bcache_debug.c \
	kernel/fs/fat/fat.c kernel/fs/fat/fat_dir.c kernel/fs/fat/fat_file.c kernel/fs/fat/fat_alloc.c kernel/fs/fat/fat_debug.c \
	kernel/ipc/ringbuf.c kernel/ipc/pipe.c kernel/tty/tty.c kernel/tty/kbd.c kernel/tty/console.c \
	kernel/proc/signal.c kernel/proc/proc_table.c kernel/proc/fork.c kernel/proc/thread_user.c kernel/proc/mm_clone.c \
	kernel/mm/addrspace.c kernel/mm/vma.c kernel/mm/page.c kernel/mm/cow.c kernel/mm/mmap.c kernel/mm/faults_vm.c kernel/mm/pagecache.c kernel/mm/anon.c kernel/mm/filemap.c kernel/mm/tlb.c kernel/mm/mm_debug.c

KERNEL_ASM_SRCS := kernel/entry.asm kernel/arch/x86/isr_stubs.asm kernel/arch/x86/ring3.asm kernel/arch/x86/syscall_stub.asm kernel/sched/context_switch.asm

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
	$(QEMU) -drive format=raw,file=$(IMAGE),if=ide,index=0 -drive format=raw,file=$(DATA_IMAGE),if=ide,index=1

clean:
	rm -rf $(BUILD_DIR)

$(DATA_IMAGE): tools/mkfatdisk.py
	mkdir -p $(BUILD_DIR)
	python3 tools/mkfatdisk.py $(DATA_IMAGE)
