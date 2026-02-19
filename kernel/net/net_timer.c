#include "net.h"
#include "tcp/tcp.h"
#include "../sched/sched.h"
#include "../time/time.h"

void net_timer_poll(void){
    tcp_set_now_ticks(time_now_ticks());
    tcp_timer_tick();
}
