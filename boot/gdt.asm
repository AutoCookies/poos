; Boot-time GDT (flat 32-bit segments)

gdt_start:
    dq 0x0000000000000000        ; null descriptor
    dq 0x00CF9A000000FFFF        ; code: base 0, limit 4GiB, ring 0
    dq 0x00CF92000000FFFF        ; data: base 0, limit 4GiB, ring 0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
