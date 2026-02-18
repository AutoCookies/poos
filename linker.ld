ENTRY(_start)

KERNEL_VIRT_BASE = 0xC0000000;
KERNEL_PHYS_BASE = 0x00100000;

SECTIONS
{
    . = KERNEL_VIRT_BASE + KERNEL_PHYS_BASE;

    __kernel_virt_start = .;
    __kernel_phys_start = . - KERNEL_VIRT_BASE;

    .text : AT(ADDR(.text) - KERNEL_VIRT_BASE)
    {
        *(.text*)
    }

    .rodata ALIGN(4K) : AT(ADDR(.rodata) - KERNEL_VIRT_BASE)
    {
        *(.rodata*)
    }

    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VIRT_BASE)
    {
        *(.data*)
    }

    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VIRT_BASE)
    {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    }

    __kernel_virt_end = .;
    __kernel_phys_end = . - KERNEL_VIRT_BASE;
}
