#include "tcp.h"

void vga_write(const char*);

#ifndef TCP_DEBUG
#define TCP_DEBUG 0
#endif

void tcp_debug_log(const char* msg){
#if TCP_DEBUG
    vga_write(msg);
#else
    (void)msg;
#endif
}
