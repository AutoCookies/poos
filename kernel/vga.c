#include "types.h"

#define DEBUGCON_PORT 0xE9

static volatile u16* const vga_buffer = (volatile u16*)VGA_TEXT_BUFFER;
static const u8 VGA_WIDTH = 80;
static const u8 VGA_HEIGHT = 25;
static u8 row = 0;
static u8 col = 0;
static u8 color = 0x0F;

static u16 vga_entry(char c) {
    return (u16)color << 8U | (u8)c;
}

static void scroll_if_needed(void) {
    if (row < VGA_HEIGHT) {
        return;
    }

    for (u32 r = 1; r < VGA_HEIGHT; ++r) {
        for (u32 c = 0; c < VGA_WIDTH; ++c) {
            vga_buffer[(r - 1) * VGA_WIDTH + c] = vga_buffer[r * VGA_WIDTH + c];
        }
    }

    for (u32 c = 0; c < VGA_WIDTH; ++c) {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + c] = vga_entry(' ');
    }

    row = VGA_HEIGHT - 1;
}

void vga_init(void) {
    row = 0;
    col = 0;
    color = 0x0F;

    for (u32 i = 0; i < (u32)VGA_WIDTH * VGA_HEIGHT; ++i) {
        vga_buffer[i] = vga_entry(' ');
    }
}

void vga_set_color(u8 fg_bg) {
    color = fg_bg;
}

static void debugcon_putc(char c) {
    outb(DEBUGCON_PORT, (u8)c);
}

void vga_putc(char c) {
    debugcon_putc(c);

    if (c == '\n') {
        col = 0;
        ++row;
        scroll_if_needed();
        return;
    }

    vga_buffer[row * VGA_WIDTH + col] = vga_entry(c);
    ++col;

    if (col >= VGA_WIDTH) {
        col = 0;
        ++row;
        scroll_if_needed();
    }
}

void vga_write(const char* s) {
    while (*s != '\0') {
        vga_putc(*s++);
    }
}

void vga_write_u32(u32 value) {
    char buf[11];
    u32 idx = 0;

    if (value == 0U) {
        vga_putc('0');
        return;
    }

    while (value > 0U && idx < sizeof(buf)) {
        buf[idx++] = (char)('0' + (value % 10U));
        value /= 10U;
    }

    while (idx > 0U) {
        vga_putc(buf[--idx]);
    }
}

void vga_write_hex(u32 value) {
    vga_write("0x");
    for (i32 i = 7; i >= 0; --i) {
        u8 nibble = (u8)((value >> (u32)(i * 4)) & 0xFU);
        char c = (nibble < 10U) ? (char)('0' + nibble) : (char)('A' + nibble - 10U);
        vga_putc(c);
    }
}
