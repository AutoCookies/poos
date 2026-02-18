#ifndef POOS_IPC_RINGBUF_H
#define POOS_IPC_RINGBUF_H
#include "../types.h"
struct ringbuf { u8* data; u32 cap; u32 r; u32 w; u32 len; };
void ringbuf_init(struct ringbuf* rb, u8* data, u32 cap);
u32 ringbuf_write(struct ringbuf* rb, const u8* src, u32 len);
u32 ringbuf_read(struct ringbuf* rb, u8* dst, u32 len);
#endif
