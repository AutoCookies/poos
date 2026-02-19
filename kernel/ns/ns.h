#ifndef POOS_NS_NS_H
#define POOS_NS_NS_H
#include "../types.h"

enum {
    CLONE_NEWNS   = 1u << 0,
    CLONE_NEWPID  = 1u << 1,
    CLONE_NEWNET  = 1u << 2,
    CLONE_NEWUTS  = 1u << 3,
    CLONE_NEWUSER = 1u << 4,
};

u32 ns_next_id(void);
#endif
