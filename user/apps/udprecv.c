#include "../libc_min/syscall.h"
int main(void){ int s=sys_socket(AF_INET,SOCK_DGRAM,0); struct sockaddr_in_k sa={AF_INET,5555,0}; sys_bind(s,&sa,sizeof(sa)); char b[128]; for(;;){ int al=sizeof(sa); int n=sys_recvfrom(s,b,sizeof(b),0,&sa,&al); if(n>0){ sys_write(1,b,n); sys_write(1,"\n",1);} } }
