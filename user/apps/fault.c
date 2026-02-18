#include "../libc_min/syscall.h"
int main(void){ *(volatile unsigned int*)0xC0001000 = 0xDEADBEEF; for(;;){} return 0; }
