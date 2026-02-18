#include "../libc_min/syscall.h"
extern void puts_min(const char*);
int main(void){
    int fd=sys_open("/etc/motd",0); char b[64];
    if(fd<0){ puts_min("cat: open /etc/motd failed\n"); return 1; }
    for(;;){ int n=sys_read(fd,b,sizeof(b)); if(n<=0) break; sys_write(1,b,n); }
    puts_min("\n");
    sys_close(fd);
    return 0;
}
