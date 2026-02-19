#include "pbuf.h"
#include "../mem/heap.h"
#include "../mem/mem.h"

pbuf_t* pbuf_alloc(u16 len, u16 headroom){
    pbuf_t* p=(pbuf_t*)kmalloc(sizeof(pbuf_t),8); if(!p) return 0;
    p->capacity=(u16)(len+headroom); p->base=(u8*)kmalloc(p->capacity,8); if(!p->base){kfree(p);return 0;}
    p->data=p->base+headroom; p->len=len; p->headroom=headroom; p->refcnt=1; p->next=0;
    if(len) mem_set(p->data,0,len);
    return p;
}
void pbuf_ref(pbuf_t* p){ if(p) p->refcnt++; }
void pbuf_free(pbuf_t* p){ if(!p) return; if(--p->refcnt) return; kfree(p->base); kfree(p); }
void* pbuf_push(pbuf_t* p,u16 len){ if(!p||len>p->headroom) return 0; p->data-=len; p->len=(u16)(p->len+len); p->headroom=(u16)(p->headroom-len); return p->data; }
void* pbuf_pull(pbuf_t* p,u16 len){ if(!p||len>p->len) return 0; p->data+=len; p->len=(u16)(p->len-len); p->headroom=(u16)(p->headroom+len); return p->data; }
