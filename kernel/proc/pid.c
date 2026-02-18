#include "../types.h"

static u32 g_next_pid = 1;

u32 pid_alloc(void) { return g_next_pid++; }
