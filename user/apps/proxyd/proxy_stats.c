#include "proxy_cache.h"
extern int printf_min(const char*, ...);
void proxy_stats_dump(void){ struct proxy_cache_stats s; proxy_cache_stats(&s); printf_min("proxyd: mem %u/%u disk %u/%u hit=%u miss=%u evict=%u\n", s.mem_used,s.mem_cap,s.disk_used,s.disk_cap,s.hit,s.miss,s.evictions); }
