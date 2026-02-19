#include "../libc_min/syscall.h"
int main(int argc,char**argv){ if(argc<2) return 1; return sys_unlink(argv[1]); }
