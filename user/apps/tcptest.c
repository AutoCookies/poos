#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);

static int worker(void){
    char host[64]="example.com"; if(sys_netctl(3,host,11)<0) return 1; unsigned int ip=*(unsigned int*)host;
    int s=sys_socket(AF_INET,SOCK_STREAM,0); if(s<0) return 1;
    struct sockaddr_in_k sa={AF_INET,80,ip}; if(sys_connect(s,&sa,sizeof(sa))<0){ sys_sockclose(s); return 1; }
    char req[]="GET / HTTP/1.0\r\nHost: example.com\r\n\r\n";
    if(sys_send(s,req,sizeof(req)-1,0)<0){ sys_sockclose(s); return 1; }
    char b[128]; int total=0; for(;;){ int n=sys_recv(s,b,sizeof(b),0); if(n<=0) break; total+=n; }
    sys_sockclose(s);
    return total>0?0:1;
}

int main(void){
    int p[5];
    for(int i=0;i<5;i++){ int pid=sys_fork(); if(pid==0){ int rc=worker(); sys_exit(rc); } p[i]=pid; }
    int ok=0;
    for(int i=0;i<5;i++){ int st=0; sys_waitpid(p[i],&st); if(st==0) ok++; }
    printf_min("tcptest: %d/5 successful\n",ok);
    return (ok==5)?0:1;
}
