#include "../libc_min/syscall.h"
extern void puts_min(const char*);
int main(int c,char**v){ (void)v; if(c<2) return 1; if(sys_setuid(0)<0){ puts_min("su failed\n"); return 1;} return sys_execve("/bin/sh"); }
