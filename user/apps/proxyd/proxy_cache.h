#ifndef PROXY_CACHE_H
#define PROXY_CACHE_H

#include "../../libc_min/syscall.h"

#define PROXY_MEM_CACHE_CAP (8*1024*1024)
#define PROXY_DISK_CACHE_CAP (32*1024*1024)
#define PROXY_MEM_ENTRY_MAX 2048
#define PROXY_DISK_ENTRY_MAX 4096
#define PROXY_KEY_MAX 320
#define PROXY_HDR_MAX 2048
#define PROXY_SMALL_OBJ_MAX (64*1024)
#define PROXY_DISK_OBJ_MAX (1024*1024)

struct proxy_cache_stats {
    unsigned int mem_used, mem_cap;
    unsigned int disk_used, disk_cap;
    unsigned int hit, miss, evictions;
};

struct proxy_cache_item {
    unsigned int used, lru, expires;
    unsigned int key_hash;
    unsigned short key_len, hdr_len;
    unsigned int body_len;
    char key[PROXY_KEY_MAX];
    char hdr[PROXY_HDR_MAX];
    unsigned char* body;
};

struct proxy_disk_item {
    unsigned int used, lru, key_hash, expires;
    unsigned short key_len;
    unsigned int file_bytes;
    char key[PROXY_KEY_MAX];
    char file[48];
};

int proxy_cache_init(unsigned int mem_cap, unsigned int disk_cap, const char* dir);
int proxy_cache_lookup_mem(const char* key, unsigned int now, const char** hdr, int* hdr_len, const unsigned char** body, int* body_len);
int proxy_cache_store_mem(const char* key, const char* hdr, int hdr_len, const unsigned char* body, int body_len, unsigned int ttl, unsigned int now);
int proxy_cache_lookup_disk(const char* key, unsigned int now, char* out, int out_cap);
int proxy_cache_store_disk(const char* key, const char* hdr, int hdr_len, const unsigned char* body, int body_len, unsigned int ttl, unsigned int now);
void proxy_cache_stats(struct proxy_cache_stats* out);
void proxy_cache_touch_miss(void);

#endif
