#include "bcache.h"
void bcache_writeback_tick(void){ (void)bcache_sync(0); }
