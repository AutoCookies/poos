#include "../../libc_min/syscall.h"
int proxy_send_all(int fd,const char* b,int n){ int off=0; while(off<n){ int w=sys_send(fd,b+off,n-off,0); if(w<=0) return -1; off+=w; } return 0; }
