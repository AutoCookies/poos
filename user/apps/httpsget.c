#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);

int main(int argc, char** argv){
    (void)argc; (void)argv;
    printf_min("httpsget: TLS v1.3 userspace handshake is not available in this build\n");
    return 1;
}
