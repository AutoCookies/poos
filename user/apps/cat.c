#include "../libc_min/syscall.h"
int main(void){
    char b[64];
    for(;;){ int n=sys_read(0,b,sizeof(b)); if(n<=0) break; sys_write(1,b,n); }
    return 0;
}
