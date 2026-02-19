#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(void){
    int fd=sys_open("/tmp/edge.status",O_RDONLY); if(fd<0){ printf_min("FAIL\n"); return 1; }
    char b[64]; int n=sys_read(fd,b,63); sys_close(fd); if(n<=0){ printf_min("FAIL\n"); return 1; } b[n]=0;
    if(b[2]=='0'){ printf_min("OK\n"); return 0; }
    if(b[2]=='1'){ printf_min("DEGRADED\n"); return 1; }
    printf_min("FAIL\n"); return 1;
}
