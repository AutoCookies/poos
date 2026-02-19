#include "../libc_min/syscall.h"
extern int printf_min(const char*,...);
int main(void){ printf_min("uid=%d euid=%d caps=0x%x\n",sys_getuid(),sys_geteuid(),sys_capget()); return 0; }
