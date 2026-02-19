#include "net.h"
#include "tcp/tcp.h"

void vga_write(const char*);
void vga_write_u32(u32);

void net_print_tcp_stats(void){
    const struct tcp_stats* s=tcp_stats_get();
    vga_write(" tcp_active=");vga_write_u32(s->active);
    vga_write(" tx=");vga_write_u32(s->seg_tx);
    vga_write(" rx=");vga_write_u32(s->seg_rx);
    vga_write(" rexmit=");vga_write_u32(s->retransmits);
    vga_write(" drop=");vga_write_u32(s->drops);
}
