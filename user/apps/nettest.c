#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(void){
    for(int i=0;i<5;i++){
        int p=sys_fork();
        if(p==0){ sys_execve("/bin/httpget"); sys_exit(1); }
    }
    for(int i=0;i<5;i++) sys_waitpid(-1,0);
    printf_min("nettest done\n");
    return 0;
}
