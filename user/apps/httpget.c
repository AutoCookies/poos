#include "../libc_min/syscall.h"
extern int printf_min(const char*, ...);

static int slen(const char* s){int n=0;while(s[n])n++;return n;}
static int find_hdr_end(const char* b,int n){ for(int i=3;i<n;i++) if(b[i-3]=='\r'&&b[i-2]=='\n'&&b[i-1]=='\r'&&b[i]=='\n') return i+1; return -1; }

int main(void){
    char host[64]="example.com";
    if(sys_netctl(3,host,11)<0){ printf_min("dns failed\n"); return 1; }
    unsigned int ip=*(unsigned int*)host;
    int s=sys_socket(AF_INET,SOCK_STREAM,0); if(s<0){ printf_min("socket failed\n"); return 1; }
    struct sockaddr_in_k sa={AF_INET,80,ip};
    if(sys_connect(s,&sa,sizeof(sa))<0){ printf_min("connect failed\n"); sys_sockclose(s); return 1; }
    char req[]="GET / HTTP/1.0\r\nHost: example.com\r\n\r\n";
    if(sys_send(s,req,slen(req),0)<0){ printf_min("send failed\n"); sys_sockclose(s); return 1; }
    char buf[512]; int seen=0;
    for(;;){ int n=sys_recv(s,buf,sizeof(buf),0); if(n<=0) break; int off=0; if(!seen){ int e=find_hdr_end(buf,n); if(e>=0){ off=e; seen=1; } else continue; } if(off<n) sys_write(1,buf+off,n-off); }
    sys_sockclose(s);
    return 0;
}
