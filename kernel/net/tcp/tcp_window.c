#include "tcp.h"

u32 tcp_window_sendable(struct tcp_conn* c){
    if(!c) return 0;
    u32 inflight = c->snd_nxt - c->snd_una;
    return (inflight >= c->send_window) ? 0 : (u32)(c->send_window - inflight);
}
