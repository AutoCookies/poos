#include "pipe.h"
#include "ringbuf.h"
#include "../vfs/file.h"
#include "../mem/heap.h"
#include "../mem/mem.h"
#include "../sched/sched.h"

#define PIPE_CAP 4096U
struct pipe_obj { struct ringbuf rb; u8 buf[PIPE_CAP]; u32 readers; u32 writers; };
struct pipe_end { struct pipe_obj* p; int write_end; };

static int pipe_read(struct file* f, u32 off, void* buf, u32 len){ (void)off; struct pipe_end* e=(struct pipe_end*)f->priv; if(!e||e->write_end) return -1; struct pipe_obj* p=e->p; for(;;){ u32 n=ringbuf_read(&p->rb,(u8*)buf,len); if(n) return (int)n; if(p->writers==0) return 0; kthread_sleep(1); } }
static int pipe_write(struct file* f, u32 off, const void* buf, u32 len){ (void)off; struct pipe_end* e=(struct pipe_end*)f->priv; if(!e||!e->write_end) return -1; struct pipe_obj* p=e->p; if(p->readers==0) return -32; u32 done=0; while(done<len){ u32 n=ringbuf_write(&p->rb,(const u8*)buf+done,len-done); done+=n; if(done==len) break; if(p->readers==0) return -32; kthread_sleep(1);} return (int)done; }
static int pipe_close(struct file* f){ struct pipe_end* e=(struct pipe_end*)f->priv; if(!e) return 0; struct pipe_obj* p=e->p; if(e->write_end){ if(p->writers) p->writers--; } else { if(p->readers) p->readers--; }
    int freep=(p->readers==0&&p->writers==0); kfree(e); if(freep) kfree(p); return 0; }
static const struct file_ops g_pipe_rd_ops={pipe_read,0,pipe_close};
static const struct file_ops g_pipe_wr_ops={0,pipe_write,pipe_close};

int pipe_create_files(struct file** rd, struct file** wr){ struct pipe_obj* p=(struct pipe_obj*)kmalloc(sizeof(*p),8); if(!p) return -1; ringbuf_init(&p->rb,p->buf,PIPE_CAP); p->readers=1; p->writers=1; struct pipe_end* re=(struct pipe_end*)kmalloc(sizeof(*re),8); struct pipe_end* we=(struct pipe_end*)kmalloc(sizeof(*we),8); if(!re||!we){ if(re)kfree(re); if(we)kfree(we); kfree(p); return -1; } re->p=p; re->write_end=0; we->p=p; we->write_end=1; *rd=file_create_special(&g_pipe_rd_ops,re,0); *wr=file_create_special(&g_pipe_wr_ops,we,0); if(!*rd||!*wr){ if(*rd)file_put(*rd); if(*wr)file_put(*wr); return -1; } return 0; }
