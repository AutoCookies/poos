#include "../libc_min/syscall.h"
extern void puts_min(const char*);
int main(void){ puts_min("hello from /bin/hello\n"); return 0; }
