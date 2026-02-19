#include "proxy_cache.h"
#include "../../libc_min/syscall.h"
extern int printf_min(const char*, ...);

extern int proxy_parse_size(const char*);
extern int proxy_slen(const char*);
extern int proxy_fetch_once(unsigned int ip, unsigned short port, const char* host, const char* path, char* out, int out_cap, int* cacheable, int* ttl);
extern void proxy_stats_dump(void);

static int parse_u16(const char* s){ int n=0; if(!s||!*s) return -1; for(int i=0;s[i];i++){ if(s[i]<'0'||s[i]>'9') return -1; n=n*10+(s[i]-'0'); } return (n>0&&n<65536)?n:-1; }
static int arg_eq(const char* a,const char* b){ int i=0; for(;;i++){ if(a[i]!=b[i]) return 0; if(!a[i]) return 1; } }

int main(int argc, char** argv){
    int listen_port=443, up_port=8080, mem_cap=8*1024*1024, disk_cap=32*1024*1024;
    char up_host[64]="127.0.0.1"; char test_path[128]="/index.html";

    for(int i=1;i<argc;i++){
        if(arg_eq(argv[i],"--listen") && i+1<argc) listen_port=parse_u16(argv[++i]);
        else if(arg_eq(argv[i],"--upstream") && i+1<argc){
            char* s=argv[++i]; int p=0,j=0; while(s[p]&&s[p]!=':'&&j<63) up_host[j++]=s[p++]; up_host[j]=0; if(s[p]==':') up_port=parse_u16(s+p+1);
        } else if(arg_eq(argv[i],"--cache-mem") && i+1<argc) mem_cap=proxy_parse_size(argv[++i]);
        else if(arg_eq(argv[i],"--cache-disk") && i+1<argc) disk_cap=proxy_parse_size(argv[++i]);
        else if(arg_eq(argv[i],"--path") && i+1<argc){ char* p=argv[++i]; int k=0; while(p[k]&&k<127){ test_path[k]=p[k]; k++; } test_path[k]=0; }
        else if(arg_eq(argv[i],"--stats")){ proxy_stats_dump(); return 0; }
    }

    if(mem_cap<=0 || disk_cap<=0){ printf_min("proxyd: bad cache cap\n"); return 1; }
    proxy_cache_init((unsigned int)mem_cap,(unsigned int)disk_cap,"/var/cache/proxyd");

    char dns[64]; int hlen=proxy_slen(up_host); for(int i=0;i<hlen;i++) dns[i]=up_host[i];
    if(sys_netctl(3,dns,hlen)<0){ printf_min("proxyd: dns failed for %s\n",up_host); return 1; }
    unsigned int ip=*(unsigned int*)dns;

    char key[PROXY_KEY_MAX]; int kp=0; const char* pfx="GET|"; for(int i=0;pfx[i];i++) key[kp++]=pfx[i];
    for(int i=0;up_host[i]&&kp<PROXY_KEY_MAX-2;i++) key[kp++]=up_host[i]; key[kp++]='|';
    for(int i=0;test_path[i]&&kp<PROXY_KEY_MAX-1;i++) key[kp++]=test_path[i]; key[kp]=0;

    const char* hdr=0; const unsigned char* body=0; int hdr_len=0,body_len=0;
    if(proxy_cache_lookup_mem(key,(unsigned int)sys_time(),&hdr,&hdr_len,&body,&body_len)==0){
        sys_write(1,body,body_len); printf_min("\nproxyd: cache-hit mem\n"); return 0;
    }
    proxy_cache_touch_miss();

    static char resp[1024*1024+4096];
    int cacheable=0, ttl=0;
    int n=proxy_fetch_once(ip,(unsigned short)up_port,up_host,test_path,resp,sizeof(resp),&cacheable,&ttl);
    if(n<=0){ printf_min("proxyd: upstream fetch failed\n"); return 1; }
    sys_write(1,resp,n);
    if(cacheable && ttl>0){
        int he=0; for(int i=3;i<n;i++) if(resp[i-3]=='\r'&&resp[i-2]=='\n'&&resp[i-1]=='\r'&&resp[i]=='\n'){ he=i+1; break; }
        if(he>0 && n-he<=PROXY_SMALL_OBJ_MAX){ proxy_cache_store_mem(key,resp,he,(unsigned char*)resp+he,n-he,(unsigned int)ttl,(unsigned int)sys_time()); }
        else if(he>0){ proxy_cache_store_disk(key,resp,he,(unsigned char*)resp+he,n-he,(unsigned int)ttl,(unsigned int)sys_time()); }
    }
    printf_min("\nproxyd: upstream mode listen=%d (note: PoOS TCP listen syscall unavailable in this build)\n",listen_port);
    return 0;
}
