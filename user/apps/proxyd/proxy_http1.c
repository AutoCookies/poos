#include "proxy_cache.h"
#include "../../libc_min/syscall.h"

extern int proxy_slen(const char*);
extern void proxy_memcpy(void*,const void*,int);
extern int printf_min(const char*, ...);

static int find_hdr_end(const char* b,int n){ for(int i=3;i<n;i++) if(b[i-3]=='\r'&&b[i-2]=='\n'&&b[i-1]=='\r'&&b[i]=='\n') return i+1; return -1; }
static int starts_with(const char* s,const char* p){ for(int i=0;p[i];i++) if(s[i]!=p[i]) return 0; return 1; }

int proxy_fetch_once(unsigned int ip, unsigned short port, const char* host, const char* path, char* out, int out_cap, int* cacheable, int* ttl){
    int s=sys_socket(AF_INET,SOCK_STREAM,0); if(s<0) return -1;
    struct sockaddr_in_k sa={AF_INET,port,ip}; if(sys_connect(s,&sa,sizeof(sa))<0){ sys_sockclose(s); return -1; }
    char req[768]; int p=0;
    const char* m="GET "; for(int i=0;m[i];i++) req[p++]=m[i];
    for(int i=0;path[i] && p<700;i++) req[p++]=path[i];
    const char* t=" HTTP/1.1\r\nHost: "; for(int i=0;t[i];i++) req[p++]=t[i];
    for(int i=0;host[i] && p<740;i++) req[p++]=host[i];
    const char* e="\r\nConnection: close\r\n\r\n"; for(int i=0;e[i];i++) req[p++]=e[i];
    if(sys_send(s,req,p,0)!=p){ sys_sockclose(s); return -1; }
    int n=0; for(;;){ int r=sys_recv(s,out+n,out_cap-n,0); if(r<=0) break; n+=r; if(n>=out_cap) break; }
    sys_sockclose(s); if(n<=0) return -1;
    *cacheable=0; *ttl=0;
    int he=find_hdr_end(out,n); if(he>0 && starts_with(out,"HTTP/1.1 200")){
        int i=0; while(i<he){
            if(i+24<he && starts_with(out+i,"Cache-Control: max-age=")){ int v=0; i+=23; while(i<he&&out[i]>='0'&&out[i]<='9'){ v=v*10+(out[i]-'0'); i++; } if(v>0){ *cacheable=1; *ttl=v; } }
            while(i<he && !(out[i]=='\r'&&out[i+1]=='\n')) i++; i+=2;
        }
    }
    return n;
}
