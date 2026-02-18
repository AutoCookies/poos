#include "tty.h"
#include "../sched/sched.h"
#include "../arch/x86/cpu.h"
void vga_putc(char c);

static char g_buf[512];
static u32 g_len;

void tty_init(void){ g_len=0; }
void tty_push_char(char c){
    if(c=='\r') c='\n';
    if(c=='\b' || c==127){ if(g_len){ g_len--; vga_putc('\b'); vga_putc(' '); vga_putc('\b'); } return; }
    if(g_len+1<sizeof(g_buf)){ g_buf[g_len++]=c; vga_putc(c); }
}
int tty_read_line(void* buf, u32 len){
    for(;;){
        u32 flags=irq_save();
        for(u32 i=0;i<g_len;i++) if(g_buf[i]=='\n'){
            u32 n=(i+1<len)?(i+1):(len-1);
            for(u32 j=0;j<n;j++) ((char*)buf)[j]=g_buf[j];
            ((char*)buf)[n]=0;
            u32 rem=g_len-(i+1);
            for(u32 j=0;j<rem;j++) g_buf[j]=g_buf[i+1+j];
            g_len=rem; irq_restore(flags); return (int)n;
        }
        irq_restore(flags);
        kthread_sleep(1);
    }
}
