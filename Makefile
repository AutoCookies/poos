CROSS ?= i686-elf
CC := $(CROSS)-gcc
LD := $(CROSS)-ld
AS := nasm
OBJCOPY := $(CROSS)-objcopy
QEMU := qemu-system-i386

CFLAGS := -std=c11 -ffreestanding -fno-pic -fno-stack-protector -fno-builtin -Wall -Wextra -Werror -m32 -O2
LDFLAGS := -T linker.ld -nostdlib -m elf_i386

BUILD_DIR := build
BOOT_BIN := $(BUILD_DIR)/boot.bin
KERNEL_ELF := $(BUILD_DIR)/kernel.elf
KERNEL_BIN := $(BUILD_DIR)/kernel.bin
IMAGE := $(BUILD_DIR)/poos.img

KERNEL_C_SRCS := \
	kernel/kernel.c \
	kernel/gdt.c \
	kernel/idt.c \
	kernel/timer.c \
	kernel/vga.c \
	kernel/panic.c

KERNEL_ASM_SRCS := \
	kernel/entry.asm \
	kernel/isr.asm

KERNEL_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_C_SRCS)) \
	$(patsubst %.asm,$(BUILD_DIR)/%.o,$(KERNEL_ASM_SRCS))

.PHONY: build run clean

build: $(IMAGE)

$(IMAGE): $(BOOT_BIN) $(KERNEL_BIN)
	mkdir -p $(BUILD_DIR)
	dd if=/dev/zero of=$(IMAGE) bs=512 count=2880 status=none
	dd if=$(BOOT_BIN) of=$(IMAGE) conv=notrunc status=none
	dd if=$(KERNEL_BIN) of=$(IMAGE) bs=512 seek=1 conv=notrunc status=none

$(BOOT_BIN): boot/boot.asm boot/gdt.asm
	mkdir -p $(BUILD_DIR)
	$(AS) -f bin -o $@ boot/boot.asm
	@boot_size=$$(stat -c%s $@); \
	if [ $$boot_size -ne 512 ]; then \
		echo "boot sector size is $$boot_size bytes (expected 512)"; \
		exit 1; \
	fi

$(KERNEL_BIN): $(KERNEL_ELF)
	$(OBJCOPY) -O binary $< $@

$(KERNEL_ELF): $(KERNEL_OBJS) linker.ld
	$(LD) $(LDFLAGS) -o $@ $(KERNEL_OBJS)

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.asm
	mkdir -p $(dir $@)
	$(AS) -f elf32 -o $@ $<

run: $(IMAGE)
	$(QEMU) -drive format=raw,file=$(IMAGE)

clean:
	rm -rf $(BUILD_DIR)
