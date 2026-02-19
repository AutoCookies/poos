#include "../libc_min/syscall.h"
extern void puts_min(const char*);
int main(void){
    puts_min("[init] pid1 online\n");
    for(;;){
        int pid=sys_spawn("/bin/login");
        int st=0;
        if(pid>0) sys_waitpid(pid,&st);
        puts_min("[init] restarting shell\n");
        sys_sleep(250);
    }
    return 0;
}
