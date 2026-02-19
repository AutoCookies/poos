#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(void){
    printf_min("debug command pending kernel telemetry integration\n");
    return 0;
}
