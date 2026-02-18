#include "../libc_min/syscall.h"

extern void puts_min(const char* s);

int main(void) {
    puts_min("hello from user\n");
    int pid = sys_getpid();
    char msg[] = "pid=00\n";
    msg[4] = '0' + ((pid / 10) % 10);
    msg[5] = '0' + (pid % 10);
    sys_write(msg, sizeof(msg)-1);
    for (;;) {
        puts_min("u\n");
        sys_sleep(100);
        sys_yield();
    }
    return 0;
}
