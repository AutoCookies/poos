#include "proxy_cache.h"

int proxy_parse_size(const char* s){
    if(!s||!*s) return -1;
    int n=0; int i=0;
    while(s[i]>='0'&&s[i]<='9'){ n=n*10+(s[i]-'0'); i++; }
    if(i==0) return -1;
    if(s[i]=='M'||s[i]=='m') n*=1024*1024;
    else if(s[i]=='K'||s[i]=='k') n*=1024;
    else if(s[i]!=0) return -1;
    return n;
}

unsigned int proxy_hash_key(const char* s){ unsigned int h=2166136261u; for(int i=0;s&&s[i];i++){ h^=(unsigned char)s[i]; h*=16777619u; } return h? h:1; }
int proxy_str_eq(const char* a,const char* b){ int i=0; for(;;i++){ if(a[i]!=b[i]) return 0; if(!a[i]) return 1; } }
int proxy_slen(const char* s){ int n=0; while(s&&s[n]) n++; return n; }
void proxy_memcpy(void* d,const void* s,int n){ unsigned char* dd=d; const unsigned char* ss=s; for(int i=0;i<n;i++) dd[i]=ss[i]; }
void proxy_memset(void* d,int v,int n){ unsigned char* dd=d; for(int i=0;i<n;i++) dd[i]=(unsigned char)v; }

int proxy_validate_request_line(const char* m, const char* p, int max_method, int max_path){
    int ml=proxy_slen(m), pl=proxy_slen(p);
    if(ml<=0||ml>max_method) return 400;
    if(pl<=0) return 400;
    if(pl>max_path) return 414;
    for(int i=0;i<pl;i++){ unsigned char c=(unsigned char)p[i]; if(c<32||c==127) return 400; }
    return 0;
}

int proxy_header_guard(int total, int count, int max_total, int max_count){
    if(total>max_total) return 431;
    if(count>max_count) return 431;
    return 0;
}
