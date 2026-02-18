#ifndef POOS_SYSCALL_DEFS_H
#define POOS_SYSCALL_DEFS_H

#include "../types.h"

enum {
    SYS_WRITE = 1,
    SYS_EXIT = 2,
    SYS_YIELD = 3,
    SYS_SLEEP = 4,
    SYS_GETPID = 5,
    SYS_OPEN = 6,
    SYS_CLOSE = 7,
    SYS_READ = 8,
    SYS_LSEEK = 9,
    SYS_STAT = 10,
    SYS_GETDENTS = 11,
    SYS_EXECVE = 12,
    SYS_WAITPID = 13,
    SYS_SPAWN = 14,
    SYS_FORK = 15,
    SYS_PIPE = 16,
    SYS_DUP2 = 17,
    SYS_KILL = 18,
};

#endif
