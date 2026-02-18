#ifndef POOS_SYSCALL_DEFS_H
#define POOS_SYSCALL_DEFS_H

#include "../types.h"

enum {
    SYS_WRITE = 1,
    SYS_EXIT = 2,
    SYS_YIELD = 3,
    SYS_SLEEP = 4,
    SYS_GETPID = 5,
};

#endif
