#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);
static void ipprint(unsigned int ip){ printf_min("%d.%d.%d.%d",(ip>>24)&255,(ip>>16)&255,(ip>>8)&255,ip&255); }
int main(void){ struct netinfo_u n; if(syscall3(SYS_NETCTL,1,(int)&n,sizeof(n))<0){ printf_min("ifconfig: no net\n"); return 1; } printf_min("eth0 mac %x:%x:%x:%x:%x:%x\n",n.mac[0],n.mac[1],n.mac[2],n.mac[3],n.mac[4],n.mac[5]); printf_min("ip "); ipprint(n.ip); printf_min(" mask "); ipprint(n.mask); printf_min(" gw "); ipprint(n.gw); printf_min(" dns "); ipprint(n.dns); printf_min("\n"); printf_min("rx=%d tx=%d drops=%d\n",n.rx,n.tx,n.drops); return 0; }
