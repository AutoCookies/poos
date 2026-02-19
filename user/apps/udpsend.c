#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
int main(void){ int s=sys_socket(AF_INET,SOCK_DGRAM,0); struct netinfo_u n; sys_netctl(1,&n,sizeof(n)); struct sockaddr_in_k d={AF_INET,5555,n.gw}; char m[]="hello"; int rc=sys_sendto(s,m,5,0,&d,sizeof(d)); printf_min("udpsend rc=%d\n",rc); return 0; }
