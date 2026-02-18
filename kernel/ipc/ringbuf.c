#include "ringbuf.h"
void ringbuf_init(struct ringbuf* rb, u8* data, u32 cap){ rb->data=data; rb->cap=cap; rb->r=rb->w=rb->len=0; }
u32 ringbuf_write(struct ringbuf* rb, const u8* src, u32 len){ u32 n=0; while(n<len&&rb->len<rb->cap){ rb->data[rb->w]=src[n++]; rb->w=(rb->w+1)%rb->cap; rb->len++; } return n; }
u32 ringbuf_read(struct ringbuf* rb, u8* dst, u32 len){ u32 n=0; while(n<len&&rb->len){ dst[n++]=rb->data[rb->r]; rb->r=(rb->r+1)%rb->cap; rb->len--; } return n; }
