#include "bcache.h"
void bcache_debug_dump(void){ struct bcache_stats s; bcache_get_stats(&s); }
