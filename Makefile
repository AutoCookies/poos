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
USER_HELLO_ELF := $(BUILD_DIR)/user/hello.elf
USER_FAULT_ELF := $(BUILD_DIR)/user/fault.elf

USER_HELLO_LBA := 150
USER_FAULT_LBA := 160

KERNEL_C_SRCS := \
	kernel/kernel.c kernel/gdt.c kernel/arch/x86/gdt.c kernel/arch/x86/tss.c kernel/arch/x86/idt.c \
	kernel/arch/x86/irq.c kernel/arch/x86/faults.c kernel/arch/x86/pic.c kernel/arch/x86/pit.c \
	kernel/time/time.c kernel/sched/sched.c kernel/sched/thread.c kernel/sched/runqueue.c kernel/sched/sleepq.c \
	kernel/vga.c kernel/panic.c kernel/mem/mem.c kernel/mem/e820.c kernel/mem/pmm.c kernel/mem/paging.c kernel/mem/vmm.c \
	kernel/mem/heap.c kernel/mem/mem_debug.c kernel/proc/proc.c kernel/proc/task.c kernel/proc/pid.c kernel/proc/elf32.c \
	kernel/proc/usercopy.c kernel/proc/ustack.c kernel/syscall/syscall.c kernel/syscall/sys_dispatch.c kernel/syscall/sys_impl.c

KERNEL_ASM_SRCS := kernel/entry.asm kernel/arch/x86/isr_stubs.asm kernel/arch/x86/ring3.asm kernel/arch/x86/syscall_stub.asm kernel/sched/context_switch.asm

KERNEL_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_C_SRCS)) $(patsubst %.asm,$(BUILD_DIR)/%.o,$(KERNEL_ASM_SRCS))
USER_COMMON_OBJS := $(BUILD_DIR)/user/crt0.o $(BUILD_DIR)/user/libc_min/syscall.o $(BUILD_DIR)/user/libc_min/printf_min.o

.PHONY: build run clean
build: $(IMAGE)

$(IMAGE): $(BOOT_BIN) $(KERNEL_BIN) $(USER_HELLO_ELF) $(USER_FAULT_ELF)
	mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$(IMAGE) bs=512 count=4096 status=none
	dd if=$(BOOT_BIN) of=$(IMAGE) conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$(IMAGE) bs=512 seek=1 conv=notrunc status=none
	dd if=$(USER_HELLO_ELF) of=$(IMAGE) bs=512 seek=$(USER_HELLO_LBA) conv=notrunc status=none
	dd if=$(USER_FAULT_ELF) of=$(IMAGE) bs=512 seek=$(USER_FAULT_LBA) conv=notrunc status=none

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

$(BUILD_DIR)/user/libc_min/printf_min.o: user/libc_min/printf_min.c
	mkdir -p $(dir $@)
	$(CC) $(U_CFLAGS) -c $< -o $@

$(BUILD_DIR)/user/apps/%.o: user/apps/%.c
	mkdir -p $(dir $@)
	$(CC) $(U_CFLAGS) -c $< -o $@

$(USER_HELLO_ELF): $(USER_COMMON_OBJS) $(BUILD_DIR)/user/apps/hello.o user/user.ld
	$(LD) -T user/user.ld -nostdlib -m elf_i386 -o $@ $(USER_COMMON_OBJS) $(BUILD_DIR)/user/apps/hello.o

$(USER_FAULT_ELF): $(USER_COMMON_OBJS) $(BUILD_DIR)/user/apps/fault.o user/user.ld
	$(LD) -T user/user.ld -nostdlib -m elf_i386 -o $@ $(USER_COMMON_OBJS) $(BUILD_DIR)/user/apps/fault.o

run: $(IMAGE)
	$(QEMU) -drive format=raw,file=$(IMAGE)

clean:
	rm -rf $(BUILD_DIR)
