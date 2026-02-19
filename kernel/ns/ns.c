#include "ns.h"
static u32 g_nsid = 1;
u32 ns_next_id(void){ return g_nsid++; }
