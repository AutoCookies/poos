#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(void){
    int fd=sys_open("/tmp/proxyd.stats",O_RDONLY); if(fd<0){ printf_min("proxystat: unavailable\n"); return 1; }
    char b[256]; int n=sys_read(fd,b,255); sys_close(fd); if(n<=0) return 1; b[n]=0;
    printf_min("%s",b); return 0;
}
