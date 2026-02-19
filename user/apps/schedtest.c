#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);

int main(void){
    int n = 8;
    for(int i=0;i<n;i++){
        int p=sys_fork();
        if(p==0){
            for(int k=0;k<200;k++){ for(volatile int j=0;j<50000;j++){} sys_yield(); }
            sys_exit(0);
        }
    }
    for(int i=0;i<n;i++) sys_waitpid(-1,0);
    printf_min("schedtest done\n");
    return 0;
}
