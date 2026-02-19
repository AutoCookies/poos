#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(int argc, char** argv){ (void)argc; (void)argv; printf_min("tlsprobe: TLS stack unavailable\n"); return 1; }
