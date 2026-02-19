#include "time.h"

u64 clocksource_now_ns(void) {
    return (u64)time_ticks() * 10000000ULL;
}
