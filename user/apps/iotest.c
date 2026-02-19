#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(void){
    const char* path="/tmp/iotest.txt";
    int fd=sys_open(path,O_CREAT|O_TRUNC|O_WRONLY);
    if(fd<0){ printf_min("iotest: open fail\n"); return 1; }
    for(int i=0;i<64;i++) sys_write(fd,"poos-smp\n",9);
    sys_close(fd);
    fd=sys_open(path,O_RDONLY);
    char b[64]; int n=sys_read(fd,b,sizeof(b));
    sys_close(fd);
    printf_min("iotest done bytes=%d\n",n);
    return 0;
}
