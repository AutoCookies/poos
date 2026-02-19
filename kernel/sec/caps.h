#ifndef POOS_SEC_CAPS_H
#define POOS_SEC_CAPS_H
#include "../types.h"
enum {
    CAP_NET_ADMIN = 0,
    CAP_NET_RAW = 1,
    CAP_SYS_ADMIN = 2,
    CAP_CHOWN = 3,
    CAP_DAC_OVERRIDE = 4,
    CAP_KILL = 5,
    CAP_MAX = 6
};
#define CAP_MASK_ALL ((1U<<CAP_MAX)-1U)
int caps_has(u32 eff, u32 cap);
#endif
