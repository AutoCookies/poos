#include "../libc_min/syscall.h"
int main(int argc,char**argv){ char b[256]; int n; if(argc<3) return 1; int s=sys_open(argv[1],O_RDONLY); if(s<0) return 1; int d=sys_open(argv[2],O_CREAT|O_TRUNC|O_WRONLY); if(d<0) return 1; while((n=sys_read(s,b,sizeof(b)))>0){ if(sys_write(d,b,n)!=n) break; } sys_close(s); sys_close(d); return 0; }
