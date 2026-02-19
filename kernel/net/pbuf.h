#ifndef POOS_PBUF_H
#define POOS_PBUF_H
#include "../types.h"
typedef struct pbuf {
    u8* base;
    u8* data;
    u16 len;
    u16 capacity;
    u16 headroom;
    u16 refcnt;
    struct pbuf* next;
} pbuf_t;

pbuf_t* pbuf_alloc(u16 len, u16 headroom);
void pbuf_ref(pbuf_t* p);
void pbuf_free(pbuf_t* p);
void* pbuf_push(pbuf_t* p, u16 len);
void* pbuf_pull(pbuf_t* p, u16 len);

#endif
