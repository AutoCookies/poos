#include "syscall.h"
#include <stdarg.h>

static void putc(char c) {
    sys_write(1, &c, 1);
}

static void puts_m(const char* s) {
    while (*s) putc(*s++);
}

static void putu(unsigned int n, int base) {
    char buf[32];
    int i = 0;
    if (n == 0) buf[i++] = '0';
    else {
        while (n > 0) {
            int r = n % base;
            buf[i++] = (r < 10) ? (r + '0') : (r - 10 + 'a');
            n /= base;
        }
    }
    while (i > 0) putc(buf[--i]);
}

static void puti(int n) {
    if (n < 0) {
        putc('-');
        putu((unsigned int)-n, 10);
    } else {
        putu((unsigned int)n, 10);
    }
}

int printf_min(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            if (*fmt == 's') {
                char* s = va_arg(ap, char*);
                if (s) puts_m(s); else puts_m("(null)");
            } else if (*fmt == 'd' || *fmt == 'i') {
                puti(va_arg(ap, int));
            } else if (*fmt == 'u') {
                putu(va_arg(ap, unsigned int), 10);
            } else if (*fmt == 'x') {
                putu(va_arg(ap, unsigned int), 16);
            } else if (*fmt == 'o') {
                putu(va_arg(ap, unsigned int), 8);
            } else if (*fmt == 'c') {
                putc((char)va_arg(ap, int));
            } else if (*fmt == '%') {
                putc('%');
            } else {
                putc('%');
                putc(*fmt);
            }
        } else {
            putc(*fmt);
        }
        fmt++;
    }
    va_end(ap);
    return 0;
}

static int slen(const char* s){int n=0;while(s[n])n++;return n;}
void puts_min(const char* s){sys_write(1,s,slen(s));}

