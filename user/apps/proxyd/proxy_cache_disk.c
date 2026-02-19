#include "proxy_cache.h"
extern unsigned int proxy_hash_key(const char*);
extern int proxy_str_eq(const char*,const char*);
extern int proxy_slen(const char*);
extern void proxy_memcpy(void*,const void*,int);
extern void proxy_memset(void*,int,int);

static struct proxy_disk_item g_disk[PROXY_DISK_ENTRY_MAX];
static unsigned int g_disk_cap=PROXY_DISK_CACHE_CAP, g_disk_used, g_lru=1;
static char g_dir[64]="/var/cache/proxyd";

static void u32hex(unsigned int x,char* o){ static const char* h="0123456789abcdef"; for(int i=0;i<8;i++){ o[7-i]=h[x&15]; x>>=4; } o[8]=0; }
static int mkpath(const char* key,char* out,int cap){ int d=proxy_slen(g_dir), p=0; if(d+10>=cap) return -1; for(int i=0;i<d;i++) out[p++]=g_dir[i]; if(p && out[p-1]!='/') out[p++]='/'; unsigned int h=proxy_hash_key(key); char hx[9]; u32hex(h,hx); for(int i=0;i<8;i++) out[p++]=hx[i]; out[p]=0; return 0; }
static int disk_victim(void){ unsigned int best=0xffffffffu; int idx=-1; for(int i=0;i<PROXY_DISK_ENTRY_MAX;i++) if(g_disk[i].used && g_disk[i].lru<best){ best=g_disk[i].lru; idx=i; } return idx; }

int proxy_cache_lookup_disk(const char* key, unsigned int now, char* out, int out_cap){
    for(int i=0;i<PROXY_DISK_ENTRY_MAX;i++) if(g_disk[i].used && proxy_str_eq(g_disk[i].key,key)){
        if(g_disk[i].expires<=now){ sys_unlink(g_disk[i].file); g_disk[i].used=0; return -1; }
        int fd=sys_open(g_disk[i].file,O_RDONLY); if(fd<0) return -1;
        int n=sys_read(fd,out,out_cap); sys_close(fd); if(n<=0) return -1;
        g_disk[i].lru=g_lru++; return n;
    }
    return -1;
}

int proxy_cache_store_disk(const char* key, const char* hdr, int hdr_len, const unsigned char* body, int body_len, unsigned int ttl, unsigned int now){
    if(body_len<=0 || body_len>PROXY_DISK_OBJ_MAX || hdr_len<=0 || hdr_len>PROXY_HDR_MAX) return -1;
    if((unsigned int)(hdr_len+body_len) > g_disk_cap) return -1;
    char file[96]; if(mkpath(key,file,sizeof(file))<0) return -1;
    while(g_disk_used + (unsigned int)(hdr_len+body_len) > g_disk_cap){ int v=disk_victim(); if(v<0) break; sys_unlink(g_disk[v].file); if(g_disk_used>g_disk[v].file_bytes) g_disk_used-=g_disk[v].file_bytes; else g_disk_used=0; g_disk[v].used=0; }
    int fd=sys_open(file,O_CREAT|O_TRUNC|O_WRONLY); if(fd<0) return -1;
    if(sys_write(fd,hdr,hdr_len)!=hdr_len){ sys_close(fd); return -1; }
    if(sys_write(fd,body,body_len)!=body_len){ sys_close(fd); return -1; }
    sys_close(fd);
    int idx=-1; for(int i=0;i<PROXY_DISK_ENTRY_MAX;i++) if(!g_disk[i].used){ idx=i; break; } if(idx<0) idx=disk_victim(); if(idx<0) return 0;
    g_disk[idx].used=1; g_disk[idx].lru=g_lru++; g_disk[idx].expires=now+ttl; g_disk[idx].file_bytes=(unsigned int)(hdr_len+body_len);
    int kl=proxy_slen(key); if(kl>=PROXY_KEY_MAX) kl=PROXY_KEY_MAX-1; g_disk[idx].key_len=(unsigned short)kl; proxy_memcpy(g_disk[idx].key,key,kl); g_disk[idx].key[kl]=0;
    int fl=proxy_slen(file); if(fl>47) fl=47; proxy_memcpy(g_disk[idx].file,file,fl); g_disk[idx].file[fl]=0; g_disk_used += g_disk[idx].file_bytes;
    return 0;
}
