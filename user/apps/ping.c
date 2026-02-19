#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(void){ struct netinfo_u n; if(syscall3(SYS_NETCTL,1,(int)&n,sizeof(n))<0) return 1; int ok=0; for(int i=0;i<4;i++){ int r=syscall3(SYS_NETCTL,2,(int)&n.gw,4); if(r==0){ printf_min("reply from gw\n"); ok++; } else printf_min("timeout\n"); sys_sleep(250);} printf_min("ping: tx=4 rx=%d\n",ok); return 0; }
