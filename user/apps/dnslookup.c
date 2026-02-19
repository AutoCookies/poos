#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
static void ipprint(unsigned int ip){ printf_min("%d.%d.%d.%d",(ip>>24)&255,(ip>>16)&255,(ip>>8)&255,ip&255); }
int main(void){ char h[64]="example.com"; if(sys_netctl(3,h,11)<0){ printf_min("dns fail\n"); return 1; } unsigned int ip=*(unsigned int*)h; printf_min("example.com -> "); ipprint(ip); printf_min("\n"); return 0; }
