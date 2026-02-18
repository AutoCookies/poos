#include "paging.h"

void vga_write(const char* s);
void vga_write_hex(u32 value);

void paging_bootstrap_init(void) {
    /* bootstrap paging is established in entry.asm before kernel_main */
}

void paging_print_status(void) {
    vga_write("Paging CR0=");
    vga_write_hex(read_cr0());
    vga_write(" CR3=");
    vga_write_hex(read_cr3());
    vga_write("\n");
}
