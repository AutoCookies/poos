#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(void){
    for(int i=0;i<16;i++){
        int p=sys_fork();
        if(p==0){
            char* m=(char*)sys_mmap(0,4096,3,0,-1,0);
            if((int)m>0){ m[0]=(char)i; }
            sys_exit(0);
        }
    }
    for(int i=0;i<16;i++) sys_waitpid(-1,0);
    printf_min("mmtest done\n");
    return 0;
}
