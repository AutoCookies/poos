#include "../libc_min/syscall.h"
int main(int argc,char**argv){ if(argc<3) return 1; return sys_rename(argv[1],argv[2]); }
